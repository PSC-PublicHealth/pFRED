//
// File: Bloque.h
//

#ifndef _FRED_BLOQUE_H
#define _FRED_BLOQUE_H

/*
 * A generic, vector-like container class that:
 *  - is implementated like a deque, hence the name
 *  - uses a vector of dynamic arrays as underlying container
 *  - never invalidates iterators/pointers/references as new blocks are allocated
 *  - never shrinks; doesn't allow deletion of individual elements
 *  - reclaims items marked for recycling (instead of deleting)
 *  - supports additional arbitrary bitmasks to control iteration
 *  - thread-safe
 *  - can traverse container and apply an arbitrary functor to each (possibly in parallel)
 */

#include <deque>
#include <map>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <cstdarg>
#include <stdint.h>

/*
 * use SSE2 (8 128 XMM registers)
 */

#define numRegisters 16
#define registerWidth 64
#define registerBanksPerBlock 16

// registersPerBlock = numRegisters * registerBanksPerBlock
#define registersPerBlock 256
// bitsPerBlock = registersPerBlock * registerWidth
#define bitsPerBlock 16384 

typedef uint64_t BitType;

template < class ObjectType, class MaskType >
class bloque {

/*
 * private data members
 *
 */

private:

  size_t blockSize;
  size_t numItems;
  size_t endIndex;

  std::deque< ObjectType * > blockVector;
  typedef std::deque< BitType * > mask;
  mask defaultMask;
  typedef std::map< MaskType, mask > MaskMap; 
  typedef typename MaskMap::iterator MaskMapItr;
  MaskMap userMasks;
  std::map< MaskType, int > user_mask_num_items;

  struct itemPosition {
    size_t block, slot;
    
    itemPosition() {
      block = 0;
      slot = 0;
    }

    itemPosition( size_t itemIndex ) {
      block = itemIndex / bitsPerBlock;
      slot = itemIndex % bitsPerBlock;
    }

    itemPosition & operator++ () {
      ++slot;
      if ( slot >= bitsPerBlock ) {
        slot = 0;
        ++block;
      }
      return *this;
    }

    bool operator== ( itemPosition const &other ) const {
      return slot == other.slot && block == other.block;
    }
    
    bool operator!= ( itemPosition const &other ) const {
      return slot != other.slot || block != other.block;
    }

    bool operator< ( itemPosition const &other ) const {
      return block < other.block || (block == other.block && slot < other.slot);
    }

    bool operator> ( itemPosition const &other ) const {
      return block > other.block || (block == other.block && slot > other.slot);
    }

    bool operator<= ( itemPosition const &other ) const {
      return *this < other || *this == other;
    }

    bool operator>= ( itemPosition const &other ) const {
      return *this > other || *this == other;
    }

    size_t asIndex() const {
      return ( block * bitsPerBlock ) + slot;
    }

    void print() {
      std::cout << block << " " << slot << std::endl;
    }

  };
  
  std::deque< itemPosition > freeSlots;

  itemPosition firstItemPosition, lastItemPosition;
  size_t firstItemIndex, lastItemIndex;

/* **************************************************************************
 * public constructors, get/set methods *************************************
 * **************************************************************************
 */

public:

  bloque() {
    numItems = 0;
    endIndex = 0;
    init();
    addBlock();
  }

  void add_mask( MaskType maskName ) {
    #pragma omp critical(BLOQUE_ADD_MASK)   
    if ( userMasks.find( maskName ) == userMasks.end() ) {
      user_mask_num_items[ maskName ] = 0;
      for ( size_t i = 0; i < blockVector.size(); ++i ) { 
        userMasks[ maskName ].push_back( new BitType[ registersPerBlock ] );
        for ( size_t j = 0; j < registersPerBlock; ++j ) {
          userMasks[ maskName ][ i ][ j ] = 0;    
        }
      }
    }
  }

  int get_free_index() {
    if ( freeSlots.empty() ) {
      addBlock();
    }
    itemPosition freePosition = freeSlots.front();    // <---------------------------------- TODO: protect with mutex!
    if ( numItems == 0 ) {
      firstItemPosition = itemPosition();
      lastItemPosition = itemPosition();
    }
    if ( freePosition < firstItemPosition ) {
      firstItemPosition = freePosition;
    }
    if ( freePosition > lastItemPosition ) {
      lastItemPosition = freePosition;
    }
    freeSlots.pop_front();                            // <---------------------------------- TODO: protect with mutex!
    return freePosition.asIndex();
  }
 
  ObjectType * get_free_pointer( int itemIndex ) {
    assert( itemIndex >= 0 );
    size_t block = itemIndex / bitsPerBlock;
    size_t slot = itemIndex % bitsPerBlock;
    return &( blockVector[ block ][ slot ] );
  }

  size_t size() {
    return numItems;
  }

  size_t size( MaskType mask ) {
    return user_mask_num_items[ mask ];
  }

  void mark_valid_by_index( size_t index ) {
    
    itemPosition pos = itemPosition( index );
    setBit( defaultMask[ pos.block ][ pos.slot / registerWidth ], pos.slot % registerWidth );
    
    if ( pos > lastItemPosition ) {
      lastItemPosition = pos;
    }
    if ( pos < firstItemPosition ) {
      firstItemPosition = pos;
    }
    ++numItems;
  }


  /*
   * Marks item as invalid and frees slot for reallocation
   */
  void mark_invalid_by_index( size_t index ) {
    itemPosition pos = itemPosition( index );
    assert( ( defaultMask[ pos.block ][ pos.slot / registerWidth ] ) & ( (BitType) 1 << ( pos.slot % registerWidth ) ) );
    clearBit( defaultMask[ pos.block ][ pos.slot / registerWidth ], pos.slot % registerWidth );
    addSlot( index );
    if ( pos == lastItemPosition ) {
      lastItemPosition = getNextItemPosition( pos );
    }
    if ( pos == firstItemPosition ) {
      firstItemPosition = getNextItemPosition( pos );
    }
    // decrement number of items
    #pragma omp atomic
    --numItems;
    // update any user masks
    for ( MaskMapItr mit = userMasks.begin(); mit != userMasks.end(); ++mit ) {
      if ( ( (*mit).second[ pos.block ][ pos.slot / registerWidth ] ) & ( (BitType) 1 << ( pos.slot % registerWidth ) ) ) {
        // unset the bit for this index in this mask
        clearBit( (*mit).second[ pos.block ][ pos.slot / registerWidth ], ( pos.slot % registerWidth ) );
        // decrement the count for this mask; must be protected from concurrent writes
        #pragma omp atomic
        --( user_mask_num_items[ (*mit).first ] );
      }
    }
  }

  /*
   * Marks item's bit in specified user Mask
   */
  void set_mask_by_index( MaskType mask, size_t index ) {
    itemPosition pos = itemPosition( index );
    // set the bit for this index in this mask
    setBit( userMasks[ mask ][ pos.block ][ pos.slot / registerWidth ], pos.slot % registerWidth );
    // increment the count for this mask; must be protected from concurrent writes
    #pragma omp atomic
    ++( user_mask_num_items[ mask ] );
  }

  /*
   * Unmarks (clears) item's bit in specified user Mask
   */
  void clear_mask_by_index( MaskType mask, size_t index ) {
    itemPosition pos = itemPosition( index );
    // unset the bit for this index in this mask
    clearBit( userMasks[ mask ][ pos.block ][ pos.slot / registerWidth ], pos.slot % registerWidth );
    // decrement the count for this mask; must be protected from concurrent writes
    #pragma omp atomic
    --( user_mask_num_items[ mask ] );
  }
  
  /*
   * Returs true if the bit for the specified index is set in the
   * specified mask (assumes that the index is valid)
   */
  bool mask_is_set( MaskType mask, size_t index ) {
     itemPosition pos = itemPosition( index );
     assert( (       defaultMask[ pos.block ][ pos.slot / registerWidth ] ) & ( (BitType) 1 << ( pos.slot % registerWidth ) ) );
     return  ( userMasks[ mask ][ pos.block ][ pos.slot / registerWidth ] ) & ( (BitType) 1 << ( pos.slot % registerWidth ) );
  }

  /*
   * Generic, sequential 'apply' method for all items in container
   */
  template < typename Functor > 
  void apply( Functor & f ) {
    for ( iterator i = begin(); i != end(); ++i ) {
      f( *i );
    }
  }

  /*
   * Sequentially apply functor for all items in container that have specified mask set
   */
  template < typename Functor > 
  void apply( MaskType mask, Functor & f ) {
    for ( iterator i = begin(); i != end(); ++i ) {
      itemPosition & pos = i.currentItemPosition;
      if ( ( userMasks[ mask ][ pos.block ][ pos.slot / registerWidth ] ) & ( (BitType) 1 << ( pos.slot % registerWidth ) ) ) {
        f( *i );
      }
    }
  }

  /*
   * Generic, parallel 'apply' method for all items in container
   */
  template < typename Functor > 
  void parallel_apply( Functor & f ) {
    #pragma omp parallel for
    for ( size_t i = 0; i < blockVector.size(); ++i ) {
      for ( size_t j = 0; j < registersPerBlock; ++j ) {
        for ( size_t k = 0; k < registerWidth; ++k ) {
          if ( ( defaultMask[ i ][ j ] ) & ( (BitType) 1 << ( k ) ) ) {
            f( blockVector[ i ][ ( j * registerWidth ) + k ] );
          }
        }
      }
    }
  }

  /*
   * Generic, parallel 'masked apply' method for all items in container
   */
  template < typename Functor > 
  void parallel_masked_apply( MaskType m, Functor & f ) {
    mask & userMask = userMasks[ m ]; 
    #pragma omp parallel for
    for ( size_t i = 0; i < blockVector.size(); ++i ) {
      for ( size_t j = 0; j < registersPerBlock; ++j ) {
        for ( size_t k = 0; k < registerWidth; ++k ) {
          if ( ( ( defaultMask[ i ][ j ] ) & ( (BitType) 1 << ( k ) ) ) &&
               ( (    userMask[ i ][ j ] ) & ( (BitType) 1 << ( k ) ) ) ) {
            f( blockVector[ i ][ ( j * registerWidth ) + k ] );
          }
        }
      }
    }
  }

  // void parallelIndexedApply( std::vector< int > indices, Functor & f ) ...

  void sortFreeSlots() {
    // <----------------------------------------------------------------------------------- TODO: mutex/lock container!!! 
    std::sort( freeSlots.begin(), freeSlots.end() ); 
  }

  /*
   * returns the bloque position for the next valid item
   */
  itemPosition getNextItemPosition( itemPosition p ) {
    // make this more efficient by bitwise/sse examination of the bitsets (registers)
    for ( itemPosition i = itemPosition( p.asIndex() + 1 ); i <= lastItemPosition; ++i ) {
      if ( ( defaultMask[ i.block ][ i.slot / registerWidth ] ) & ( (BitType) 1 << ( i.slot % registerWidth ) ) ) {
        return itemPosition( i.asIndex() );
      }
    }
    return itemPosition( lastItemPosition.asIndex() + 1 ); // one past the last valid position
  }

  /*
   * returns the bloque position for the next valid item with the specified mask set
   */
  template < typename __MaskType >
  itemPosition getNextItemPosition( itemPosition p ) {
    // make this more efficient by bitwise/sse examination of the bitsets (registers)
    for ( itemPosition i = itemPosition( p.asIndex() + 1 ); i <= lastItemPosition; ++i ) {
      if ( ( defaultMask[ i.block ][ i.slot / registerWidth ] ) & ( (BitType) 1 << ( i.slot % registerWidth ) ) ) {
        return itemPosition( i.asIndex() );
      }
    }
    return itemPosition( lastItemPosition.asIndex() + 1 ); // one past the last valid position
  }

  ObjectType & get_item_by_position( itemPosition & pos ) {
    return blockVector[ pos.block ][ pos.slot ];
  }

  ObjectType & get_item_by_index( size_t itemIndex ) {
    size_t block = itemIndex / bitsPerBlock;
    size_t slot = itemIndex % bitsPerBlock;
    return blockVector[ block ][ slot ];
  }

  //////////// BEGIN ITERATORS ///////////////////////////
  
  /// Basic iterator for bloque (iterates over all valid items in container)

  template< class IteratedObjectType >
  struct bloque_iterator : std::iterator< std::forward_iterator_tag, IteratedObjectType > {

    IteratedObjectType & operator* () { return blq->get_item_by_position( currentItemPosition ); }

    template< class __IteratedObjectType >
    friend bool operator== ( bloque_iterator const &lhs, bloque_iterator< __IteratedObjectType > const &rhs ) {
      return lhs.currentItemPosition == rhs.currentItemPosition;
    }

    template< class __IteratedObjectType >
    friend bool operator!= ( bloque_iterator const &lhs, bloque_iterator< __IteratedObjectType > const &rhs ) {
      return lhs.currentItemPosition != rhs.currentItemPosition;
    }

    bloque_iterator & operator++ () {
      currentItemPosition = blq->getNextItemPosition( currentItemPosition );
      return *this;
    }

    template< class __IteratedObjectType >
    bool operator< ( bloque_iterator< __IteratedObjectType > const & other ) {
      return currentItemPosition < other.currentItemPosition;
    }

    //private:

    itemPosition currentItemPosition;
    bloque * blq;

    // private constructor for begin, end
    friend class bloque;
    bloque_iterator( bloque * __bloque, itemPosition __currentItemPosition ) {
      blq = __bloque;
      currentItemPosition = __currentItemPosition;
    }
  };
 
  typedef bloque_iterator< ObjectType > iterator;
  typedef bloque_iterator< ObjectType const > const_iterator;

  iterator begin() {
    return iterator( this, firstItemPosition );
  }

  iterator end() {
    return iterator( this, itemPosition( lastItemPosition.asIndex() + 1 ) );
  }

  /// Masked iterator for bloque (iterates over all valid items for which the bit for the specified mask is set)

  template< class IteratedObjectType, class IteratedMaskType >
  struct bloque_masked_iterator : std::iterator< std::forward_iterator_tag, IteratedObjectType > {

    IteratedObjectType & operator* () { return blq->get_item_by_position( currentItemPosition ); }

    template< class __IteratedObjectType, class __IteratedMaskType >
    friend bool operator== ( bloque_masked_iterator const &lhs, bloque_masked_iterator< __IteratedObjectType, __IteratedMaskType > const &rhs ) {
      return lhs.currentItemPosition == rhs.currentItemPosition;
    }

    template< class __IteratedObjectType, class __IteratedMaskType >
    friend bool operator!= ( bloque_masked_iterator const &lhs, bloque_masked_iterator< __IteratedObjectType, __IteratedMaskType > const &rhs ) {
      return lhs.currentItemPosition != rhs.currentItemPosition;
    }

    bloque_masked_iterator & operator++ () {
      currentItemPosition = blq->getNextItemPosition< IteratedMaskType >( currentItemPosition );
      return *this;
    }

    template< class __IteratedObjectType, class __IteratedMaskType >
    bool operator< ( bloque_masked_iterator< __IteratedObjectType, __IteratedMaskType > const & other ) {
      return currentItemPosition < other.currentItemPosition;
    }

    //private:

    itemPosition currentItemPosition;
    bloque * blq;

    // private constructor for begin, end
    friend class bloque;
    bloque_masked_iterator( bloque * __bloque, itemPosition __currentItemPosition ) {
      blq = __bloque;
      currentItemPosition = __currentItemPosition;
    }
  };

  // templated typedef workaround

  template< typename __MaskType >
  struct masked_iterator : bloque_masked_iterator< ObjectType, __MaskType > { };

  template< typename __MaskType >
  struct const_masked_iterator : bloque_masked_iterator< ObjectType const, __MaskType > { };

  // begin, end for masked iterators

  template< typename __MaskType >
  masked_iterator< __MaskType > begin() {
    return masked_iterator< __MaskType >( this, firstItemPosition );
  }

  template< typename __MaskType >
  const_masked_iterator< __MaskType > begin() {
    return const_masked_iterator< __MaskType >( this, firstItemPosition );
  }

  template< typename __MaskType >
  masked_iterator< __MaskType > end() {
    return masked_iterator< __MaskType >( this, itemPosition( lastItemPosition.asIndex() + 1 ) );
  }

  template< typename __MaskType >
  const_masked_iterator< __MaskType > end() {
    return const_masked_iterator< __MaskType >( this, itemPosition( lastItemPosition.asIndex() + 1 ) );
  }

  ///////////// END ITERATORS ////////////////////////////

  void print() {

    /*
    //firstItemPosition.print();
    //lastItemPosition.print();
    for ( size_t i = 0; i < blockVector.size(); ++i ) {
      for ( size_t j = 0; j < bitsPerBlock; ++j ) {
        std::cout << i << " " << j << " " << blockVector[i][j] << " " << (bool) ( ( defaultMask[ i ][ j / registerWidth ] ) & ( (BitType) 1 << ( j % registerWidth ) ) ) << std::endl;
      }
    }
    */ 

    for ( iterator i = begin(); i != end(); ++i ) {
      i.currentItemPosition.print();
      std::cout << *i << std::endl;
    }
    
  }

  /*
   * overloaded stream; mainly for debugging
   * decorated template names __ObjectType and __MaskType to prevent gcc 'shadowing' error
   *
  
  template < class __ObjectType, class __MaskType >
  friend std::ostream & operator<< ( std::ostream & os, const bloque< __ObjectType, __MaskType > & ) {
    std::cout << "hello there" << std::endl;
  }
  */

/* ****************************************************************
 * private methods ************************************************
 * ****************************************************************
 */

private:

  void init() {
    numItems = 0;
    blockSize = bitsPerBlock;
  }
  
  void addBlock() {
    
    size_t beginNewBlock = blockVector.size();
    size_t index = endIndex;
    blockVector.push_back( new ObjectType[ blockSize ] );
    defaultMask.push_back( new BitType[ registersPerBlock ] );

    for ( size_t i = beginNewBlock; i < blockVector.size(); ++i ) { 
      for ( MaskMapItr mit = userMasks.begin(); mit != userMasks.end(); ++mit ) {
        (*mit).second.push_back( new BitType[ registersPerBlock ] );
      }
      for ( size_t j = 0; j < registersPerBlock; ++j ) {
        defaultMask[ i ][ j ] = 0;
        for ( MaskMapItr mit = userMasks.begin(); mit != userMasks.end(); ++mit ) {
          (*mit).second[ i ][ j ] = 0;
        }
        for ( size_t k = 0; k < registerWidth; ++k ) {
          addSlot( index );
          //std::cout << "index added: " << index << std::endl;
          ++index;
        }
      }
    }
    endIndex += blockSize;
  }

  void addSlot( size_t slot_index ) {
    freeSlots.push_back( itemPosition( slot_index ) );        
  }
  
  void setBit( BitType & registerSet, size_t bit ) {
    #pragma omp atomic
    registerSet |= ( (BitType) 1 << bit );
  }
  
  void clearBit( BitType & registerSet, size_t bit ) {  
    #pragma omp atomic
    registerSet &= ~( (BitType) 1 << bit);
  }
    
  void flipBit( BitType & registerSet, size_t bit ) {
    #pragma omp atomic
    registerSet ^= (BitType) 1 << bit;
  }

};


#endif

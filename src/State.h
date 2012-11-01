//
// File: State.h
//

#ifndef _FRED_STATE_H
#define _FRED_STATE_H

/*
 * A template class that abstracts the notion of thread-local storage
 * and thread-specific execution.
 *
 * J. DePasse <jvd10@pitt.edu> 
 *
 * 2012 08 15
 *
 */

#include "Global.h"

// Template parameters:
//  Typ => the object type
//  Dim => the number of dimensions (threads)

template< class Typ >
class State {

  int Dim;
  Typ * state_array;

  public:

  State( int dim ) {
    Dim = dim;
    state_array = new Typ[ dim ];
  }

  State() { };

  ~State() { }

  Typ & operator() ( int dim_num ) {
    return state_array[ dim_num ];
  }

  Typ & operator() (             ) {
    return state_array[ fred::omp_get_thread_num() % Dim ];
  }

  int size() {
    return Dim;
  }

  // applies supplied functor to each state in array; main 
  // intended use is as a reduction/aggregation operation
  template< typename Fnc >
  void apply( Fnc & fnc ) {
    for ( int i = 0; i < Dim; ++i ) {
      fnc( state_array[ i ] );
    }
  }

  // concurrently applies supplied functor to each state in array
  // intended use is as a reduction/aggregation operation
  template< typename Fnc >
  void parallel_apply( Fnc & fnc ) {
    #pragma omp parallel for
    for ( int i = 0; i < Dim; ++i ) {
      fnc( state_array[ i ] );
    }
  }

  // these methods must be implemented in the Typ class
  void clear() {
    for ( int i = 0; i < Dim; ++i ) {
      state_array[ i ].clear();
    }
  }

  void reset() {
    for ( int i = 0; i < Dim; ++i ) {
      state_array[ i ].reset();
    }
  }

};





#endif

/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: PRandom_Streams.cc
//

#include "PRandom_Streams.h"

PRandom_Streams::PRandom_Streams( unsigned numStreams, unsigned long _seed ) {

  streamsPerBlock = numStreams;

  prngs.clear();

  seed = _seed;

  for ( unsigned i = 1; i <= numStreams; ++i ) {
    prngs.push_back( new PRandom( (unsigned long) i + seed ) );
  }
  
}


/** 
 * selects a random number stream based on where the blockItem
 * falls within the blockRangeSize.  The grainsize is 
 * the same as that used by parallel_for to divide a blocked_range.
 * The blocked_range is recursively split as long as it is larger
 * than the grainsize.  Therefore (if using the simple_partitioner)
 * a block is always going to be:
 *
 *          1/2 grainsize <= blocksize <= grainsize
 */

PRandom * PRandom_Streams::getStream( unsigned firstItem, unsigned blockRangeSize, unsigned grainSize ) {

  prngsMtxT::scoped_lock lock( prngsMtx );  

  // make sure that there are at least enough streams to provide a
  // different one for each block in the range
  while ( prngs.size() <= ( blockRangeSize / ( grainSize / 2 ) ) ) {
    unsigned i = prngs.size() + 1;    
    prngs.push_back( new PRandom( (unsigned long) i + seed ) );
  }

  size_t s = firstItem / ( grainSize / 2 );
  //cout << "returning stream number " << s << endl;

  return prngs[s];
}





/*
   Copyright 2009 by the University of Pittsburgh
   Licensed under the Academic Free License version 3.0
   See the file "LICENSE" for more information
   */

//
//
// File: PRandom_Streams.h
//

#ifndef _FRED_PRANDOM_STREAMS_H
#define _FRED_PRANDOM_STREAMS_H

#include <math.h>
#include <iostream>
#include <vector>
#include "PRandom.h"
#include <tbb/tbb.h>

using namespace std;

typedef tbb::spin_mutex prngsMtxT;

// PARALLEL RANDOM NUMBER GENERATOR UTILITIES
// 
// Using Mersenne Twister MT19937 by T. Nishimura and M. Matsumoto
// See mt19937ar.c for acknowledgements
// Adapted by J. DePasse, April 2011

class PRandom_Streams {

  public:

    PRandom_Streams( unsigned numStreams, unsigned long _seed );
    
    PRandom * getStream( unsigned firstItem, unsigned blockRangeSize, unsigned grainSize );

  private:

    unsigned long seed;
    vector< PRandom * > prngs;    
    unsigned streamsPerBlock;
    prngsMtxT prngsMtx;
};

#endif // _FRED_PRANDOM_STREAMS_H


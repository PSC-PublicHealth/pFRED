/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: PRandom.cc
//

// Mersenne Twister by Nishimura and Matsumoto

/* 
 A C-program for MT19937, with initialization improved 2002/1/26.
 Coded by Takuji Nishimura and Makoto Matsumoto.
 
 Before using, initialize the state by using init_genrand(seed)  
 or init_by_array(init_key, key_length).
 
 Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 All rights reserved.                          
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 3. The names of its contributors may not be used to endorse or promote 
 products derived from this software without specific prior written 
 permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
 Any feedback is very welcome.
 http://www.math.keio.ac.jp/matumoto/emt.html
 email: matumoto@math.keio.ac.jp
 */

#include <vector>
#include "Random.h"
#include <stdio.h>
#include "PRandom.h"

/* Period parameters ... as defined in Random.h */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   // constant vector a
#define UPPER_MASK 0x80000000UL // most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // least significant r bits
#define TWOPI (2.0*3.141592653589)


PRandom::PRandom( unsigned long seed ) {
  mt = new unsigned long [N]; // the array for the state vector
  mti = N + 1; // mti==N+1 means mt[N] is not initialized
  mag01 = new unsigned long[2];
  init_genrand( seed );
}

/* initializes mt[N] with a seed */
void PRandom::init_genrand(unsigned long s) {

  mt[0]= s & 0xffffffffUL;
  for (mti=1; mti<N; mti++) {
    mt[mti] = 
    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
    mt[mti] &= 0xffffffffUL;
  }
}

void PRandom::init_by_array(unsigned long init_key[], int key_length) {

  int i, j, k;
  init_genrand(19650218UL);
  i=1; j=0;
  k = (N>key_length ? N : key_length);
  for (; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
    + init_key[j] + j; /* non linear */
    mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
    i++; j++;
    if (i>=N) { mt[0] = mt[N-1]; i=1; }
    if (j>=key_length) j=0;
  }
  for (k=N-1; k; k--) {
    mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
    - i; /* non linear */
    mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
    i++;
    if (i>=N) { mt[0] = mt[N-1]; i=1; }
  }
  
  mt[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array 
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long PRandom::genrand_int32() {

  unsigned long y;
  mag01[0] = 0x0UL;
  mag01[1] = MATRIX_A;
  // mag01[x] = x * MATRIX_A  for x=0,1
  
  if (mti >= N) { // generate N words at one time 
    int kk;
    
    if (mti == N+1)   // if init_genrand() has not been called, 
      init_genrand(5489UL); // a default initial seed is used 
    
    for (kk=0;kk<N-M;kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    for (;kk<N-1;kk++) {
      y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
      mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
    }
    y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
    mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];
    
    mti = 0;
  }
  
  y = mt[mti++];
  
  // Tempering
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);
  
  return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long PRandom::genrand_int31() {
  return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double PRandom::genrand_real1() {
  return genrand_int32()*(1.0/4294967295.0); 
  /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double PRandom::genrand_real2() {
  return genrand_int32()*(1.0/4294967296.0); 
  /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double PRandom::genrand_real3() {
  return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); 
  // divided by 2^32
}

/* generates a random number on [0,1) with 53-bit resolution*/
double PRandom::genrand_res53() { 
  unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6; 
  return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 

/// Real versions above are due to Isaku Wada, 2002/01/09 added
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/// FRED Utility Functions Below //////////////////////////////

int PRandom::draw_from_distribution(int n, double *dist) {
  double r = random();
  int i = 0;
  while (i <= n && dist[i] < r) { i++; }
  if (i <= n) { return i; }
  else {
    printf("Help! draw from distribution failed.\n");
    printf("Is distribution properly formed? (should end with 1.0)\n");
    for (int i = 0; i <= n; i++) {
      printf("%f ", dist[i]);
    }
    printf("\n");
    return -1;
  }
}

double PRandom::draw_exponential(double lambda) {
  double u = random();
  return (-log(u)/lambda);
}

double PRandom::draw_standard_normal() {
  // Box-Muller method
  double U = random();
  double V = random();
  return (sqrt(-2.0*log(U)) * cos(TWOPI*V));
}

double PRandom::draw_normal(double mu, double sigma) {
  return mu + sigma * draw_standard_normal();
}

int PRandom::draw_from_cdf(double *v, int size) {
  double r = random();
  int top = size-1;
  int bottom = 0;
  int s = top / 2;
  while (bottom <= top) {
    if (r <= v[s]) {
      if (s == 0 || r > v[s-1])
	return s;
      else {
	top = s-1;
      }
    }
    else { // r > v[s]
      if (s == size-1)
	return s;
      if (r < v[s+1])
	return s+1;
      else {
	bottom = s+1;
      }
    }
    s = bottom + (top-bottom)/2;
  }
  // assert(bottom <= top);
  return -1;
}

int PRandom::draw_from_cdf_vector(const vector <double>& v) {
  int size = v.size();
  double r = random();
  int top = size-1;
  int bottom = 0;
  int s = top / 2;
  while (bottom <= top) {
    if (r <= v[s]) {
      if (s == 0 || r > v[s-1])
	return s;
      else {
	top = s-1;
      }
    }
    else { // r > v[s]
      if (s == size-1)
	return s;
      if (r < v[s+1])
	return s+1;
      else {
	bottom = s+1;
      }
    }
    s = bottom + (top-bottom)/2;
  }
  // assert(bottom <= top);
  return -1;
}


int PRandom::draw_poisson(double lambda) {
  if (lambda <= 0.0) return 0;
  else {
    double L = exp(-lambda);
    int k = 0;
    double p = 1.0;
    do {
      p *= random();
      k++;
    } while (p > L);
    return k-1;
  }
}






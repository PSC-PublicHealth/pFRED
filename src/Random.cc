/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Random.cc
//
#include <limits>
#include <vector>
#include <bitset>
#include <stdio.h>

#include "Random.h"

using namespace std;

// static
int draw_from_distribution(int n, double *dist) {
  double r = RANDOM();
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

double draw_exponential(double lambda) {
  double u = RANDOM();
  return (-log(u)/lambda);
}

#define TWOPI (2.0*3.141592653589)

double draw_standard_normal() {
  // Box-Muller method
  double U = RANDOM();
  double V = RANDOM();
  return (sqrt(-2.0*log(U)) * cos(TWOPI*V));
}

double draw_normal(double mu, double sigma) {
  return mu + sigma * draw_standard_normal();
}


int draw_from_cdf(double *v, int size) {
  double r = RANDOM();
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

int draw_from_cdf_vector(const vector <double>& v) {
  int size = v.size();
  double r = RANDOM();
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

using namespace std;
/*
   algorithm poisson random number (Knuth):
init:
Let L = exp(−lambda), k = 0 and p = 1.
do:
k = k + 1.
Generate uniform random number u in [0,1] and let p = p × u.
while p > L.
return k − 1.
*/

int draw_poisson(double lambda) {
  if (lambda <= 0.0) return 0;
  else {
    double L = exp(-lambda);
    int k = 0;
    double p = 1.0;
    do {
      p *= RANDOM();
      k++;
    } while (p > L);
    return k-1;
  }
}

double binomial_coefficient( int n, int k ) {
  if ( k < 0 ||  k > n ) return 0;
  if ( k > n - k ) k = n - k;
  double c = 1;
  for ( int i = 0; i < k; ++i ) {
    c = c * ( n - ( k - ( i + 1 ) ) );
    c = c / ( i + 1 );
  }
  return c;
}

void build_binomial_cdf( double p, int n, std::vector< double > & cdf ) {
  for ( int i = 0; i <= n; ++i ) {
    double prob = 0.0;
    for ( int j = 0; j <= i; ++j ) {
      prob += binomial_coefficient( n, i ) 
        * pow( 10, ( ( i * log10( p ) ) + ( ( n - 1 ) * log10( 1 - p ) ) ) );
    }
    if ( i > 0 ) {
      prob += cdf.back();
    }
    if ( prob < 1 ) {
      cdf.push_back( prob );
    }
    else {
      cdf.push_back( 1.0 );
      break;
    }
  }
  cdf.back() = 1.0;
}

void sample_range_without_replacement( int N, int s, int * result ) {
  std::vector< bool > selected( N, false );
  for ( int n = 0; n < s; ++n ) {
    int i = IRAND( 0, N - 1 );
    if ( selected[ i ] ) {
      if ( i < N - 1 && !( selected[ i + 1 ] ) ) {
        ++i;
      }
      else if ( i > 0 && !( selected[ i - 1 ] ) ) {
        --i;
      }
      else {
        --n;
        continue;
      }
    }
    selected[ i ] = true;
    result[ n ] = i;
  }
}


/////////////////////////////////////////////////////////////////

RNG_State< 8192, 1024, 1024, 2048 > rng_state[ Global::MAX_NUM_THREADS ];

void RNG::init( int seed ) {
  dsfmt_gv_init_gen_rand( seed );
  for (int i = 0; i < Global::MAX_NUM_THREADS; ++i) {
    rng_state[ i ] = RNG_State< 8192, 1024, 1024, 2048 >();
    rng_state[ i ].init( dsfmt_gv_genrand_uint32() );
  }
}

double RNG::random_double() {
  assert( fred::omp_get_thread_num() < Global::MAX_NUM_THREADS );
  return rng_state[ fred::omp_get_thread_num() ].random_double();
}

unsigned char RNG::random_char() {
  assert( fred::omp_get_thread_num() < Global::MAX_NUM_THREADS );
  return rng_state[ fred::omp_get_thread_num() ].random_char();
}

int RNG::random_int_0_7() {
  assert( fred::omp_get_thread_num() < Global::MAX_NUM_THREADS );
  int int_0_7 = ( (int) ( rng_state[ fred::omp_get_thread_num() ].random_char() >> 5 ) );
  assert( int_0_7 < 8 && int_0_7 >= 0 );
  return int_0_7;
}

void RNG::refresh_all_buffers() {
  assert( fred::omp_get_thread_num() < Global::MAX_NUM_THREADS );
  #pragma omp parallel for
  for ( int t = 0; t < fred::omp_get_max_threads(); ++t ) {
    rng_state[ t ].refresh_all_buffers();
  }
}



















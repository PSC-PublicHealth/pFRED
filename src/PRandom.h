/*
   Copyright 2009 by the University of Pittsburgh
   Licensed under the Academic Free License version 3.0
   See the file "LICENSE" for more information
   */

//
//
// File: PRandom.h
//

#ifndef _FRED_PRANDOM_H
#define _FRED_PRANDOM_H

#include <math.h>
#include <vector>

using namespace std;

// PARALLEL RANDOM NUMBER GENERATOR UTILITIES
// 
// Using Mersenne Twister MT19937 by T. Nishimura and M. Matsumoto
// See mt19937ar.c for acknowledgements
// Adapted by J. DePasse, April 2011

//#define INIT_RANDOM(SEED)   init_genrand(SEED)
//#define IRAND(LOW,HIGH) ((int)((LOW)+(int)(((HIGH)-(LOW)+1)*RANDOM())))
//#define URAND(LOW,HIGH) ((double)((LOW)+(((HIGH)-(LOW))*RANDOM())))

class PRandom {

  public:

    PRandom( unsigned long seed );
    ~PRandom();

    void init_genrand(unsigned long s);
    void init_by_array(unsigned long init_key[], int key_length);

    /// generates a random number on [0,0xffffffff]-interval
    unsigned long genrand_int32();
    /// generates a random number on [0,0x7fffffff]-interval    
    long genrand_int31();
    /// generates a random number on [0,1]-real-interval
    double genrand_real1();
    /// generates a random number on [0,1)-real-interval
    double genrand_real2();
    /// generates a random number on (0,1)-real-interval
    double genrand_real3();
    /// generates a random number on [0,1) with 53-bit resolution
    double genrand_res53();

    int draw_poisson(double lambda);
    double draw_exponential(double lambda);
    int draw_from_distribution(int n, double *dist);
    double draw_standard_normal();
    double draw_normal(double mu, double sigma);
    int draw_from_cdf(double *v, int size);
    int draw_from_cdf_vector(const vector <double>& v);

    double random() { return genrand_real2(); }
    int irand( int LOW, int HIGH) { return ( (int) ( (LOW) + (int) (((HIGH)-(LOW)+1)*random()))); }
    double urand(double LOW, double HIGH) { return ( (double) ((LOW) + ( ((HIGH)-(LOW)) * random())) ); }

    template <typename T> void FYShuffle( vector <T> &array){
      int m,randIndx;
      T tmp;
      unsigned int n = array.size();
      m=n;
      while (m > 0){
        randIndx = (int)(random()*n);
        m--;
        tmp = array[m];
        array[m] = array[randIndx];
        array[randIndx] = tmp;
      }
    }

  private:

    unsigned long * mag01;
    unsigned long * mt;
    int mti;

};


#endif // _FRED_PRANDOM_H


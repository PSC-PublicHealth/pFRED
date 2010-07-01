/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Random.h
//

#ifndef _FRED_RANDOM_H
#define _FRED_RANDOM_H

#include <math.h>
#include <vector>

using namespace std;

//*************** RANDOM NUMBER GENERATOR UTILITIES

/* Using Marseinne Twister MT19937 by T. Nishimura and M. Matsumoto */
/* See mt19937ar.c for acknowledgements */

double genrand_real2(void);
void init_genrand(unsigned long s);
#define INIT_RANDOM(SEED)   init_genrand(SEED)
#define RANDOM()        genrand_real2()
#define IRAND(LOW,HIGH) ((int)((LOW)+(int)(((HIGH)-(LOW)+1)*RANDOM())))
#define URAND(LOW,HIGH) ((double)((LOW)+(((HIGH)-(LOW))*RANDOM())))

int draw_poisson(double lambda);
double draw_exponential(double lambda);
int draw_from_distribution(int n, double *dist);

template <typename T> 
void FYShuffle( vector <T> &array){
  int m,randIndx;
  T tmp;
  unsigned int n = array.size();
  m=n;
  while (m > 0){
    randIndx = (int)(RANDOM()*n);
    m--;
    tmp = array[m];
    array[m] = array[randIndx];
    array[randIndx] = tmp;
  }
}

#endif // _FRED_RANDOM_H

//
//
// File: Random.hpp
//

#ifndef _SYNDEM_RANDOM_H
#define _SYNDEM_RANDOM_H

#include <math.h>

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

#endif // _SYNDEM_RANDOM_H

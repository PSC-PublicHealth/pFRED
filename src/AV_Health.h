/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Status.h
//

#ifndef _FRED_AVSTATUS_H
#define _FRED_AVSTATUS_H

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "Random.h"

class Antiviral;
class Antivirals;
class Health;

using namespace std;

class AV_Health{
 public:
  // Creation Operations
  AV_Health(void);
  AV_Health(int _av_day, Antiviral* _AV, Health* _health);
  
  //Access Members 
  int get_av_start_day(void)        const {return av_day;}
  int get_av_end_day(void)          const {return av_end_day;}
  Health* get_health(void)          const {return health;}
  int get_strain(void)              const {return strain;}
  
  int is_on_av(int day)    {
    if((day >= av_day) && (day <= av_end_day)) return 1;
    else return 0;
  }
  
  int is_effective(void){
    if(av_end_day !=-1) return 1;
    else return 0;
  }

  //Utility Functions
  void update(int day);
  void print(void);
  void printTrace(void);
  
  
 private:
  int av_day;           // Day on which the AV treatment starts
  int av_end_day;       // Day on which the AV treatment ends
  Health* health;       // Pointer to the health class for agent
  int strain;           // Strain for this AV
  Antiviral* AV;        // Pointer to the AV the person took
};

#endif

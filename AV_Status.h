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

class AV_Status{
 public:
  // Creation Operations
  AV_Status(void);
  AV_Status(int day, Antiviral* av, Health* h);
  
  //Access Members 
  int get_av_start_day(void)     {return av_day;}
  int get_av_end_day(void) {return av_end_day;}
  Health* get_health(void) {return health;}
  int get_strain(void) {return strain;}
  
  int is_on_av(int day)    {
    if((day >= av_day) && (day <= av_end_day)) return 1;
    else return 0;
  }
  
  int is_effective(void){
    if(av_end_day !=-1) return 1;
    else return 0;
  }

  //Utility Functions
  void print(void);
  void printTrace(void);
  void update(int day);
  
 private:
  int av_day;
  int av_end_day;
  Health* health;
  int strain;
  Antiviral* AV; 
};

#endif

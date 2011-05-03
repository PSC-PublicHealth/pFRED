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
  AV_Health();
  AV_Health(int _av_day, Antiviral* _AV, Health* _health);
  
  //Access Members 
  virtual int get_av_start_day()        const {return av_day;}
  virtual int get_av_end_day()          const {return av_end_day;}
  virtual Health* get_health()          const {return health;}
  virtual int get_disease()              const {return disease;}
  virtual Antiviral *get_antiviral()		const { return AV; }
	
  virtual int is_on_av(int day) const {
    if((day >= av_day) && (day <= av_end_day)) return 1;
    else return 0;
  }
  
  virtual int is_effective() const {
    if(av_end_day !=-1) return 1;
    else return 0;
  }
  
  //Utility Functions
  virtual void update(int day);
  virtual void print() const;
  virtual void printTrace() const;
  
  
private:
  int av_day;           // Day on which the AV treatment starts
  int av_end_day;       // Day on which the AV treatment ends
  Health* health;       // Pointer to the health class for agent
  int disease;           // Disease for this AV
  Antiviral* AV;        // Pointer to the AV the person took
};

#endif

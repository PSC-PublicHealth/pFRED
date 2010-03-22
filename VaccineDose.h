/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: VaccineDose.h
//

#ifndef _FRED_VACCINEDOSE_H
#define _FRED_VACCINEDOSE_H

#include "AgeMap.h"

class Vaccine_Dose {
 public:
  Vaccine_Dose(AgeMap &eff, AgeMap &effdel, int dbd);
  
  AgeMap* get_efficacy_map(void)       { return &Efficacy;}
  AgeMap* get_efficacy_delay_map(void) { return &EfficacyDelay;}
  
  double  get_efficacy(int age)         { return Efficacy.find_value(age);  }
  double  get_efficacy_delay(int age)   { return EfficacyDelay.find_value(age); }
  int     get_days_between_doses(void)  { return DaysBetweenDoses; }
  
  int     is_within_age(int age);
  //Utility Functions
  void print(void);
  
  
 private:
  AgeMap Efficacy;
  AgeMap EfficacyDelay;
  int    DaysBetweenDoses;
};

#endif

  
  
  

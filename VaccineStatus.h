/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: VaccineStatus.h
//

#ifndef _FRED_VACCINESTATUS_H
#define _FRED_VACCINESTATUS_H

#include <assert>
#include <stdio>
#include <iostream>

class Vaccine_Status {
 public:
  //Creation Operations
  Vaccine_Status(void){
    vaccination_day           = -1;
    vaccination_effective_day = -1;
    current_dose              =0;
  }
  
  // Access Members
  int get_vaccination_day(void)           { return vaccination_day; }
  int get_vaccination_effective_day(void) { return vaccination_effective_day; }
  Vaccine* get_vaccine(void)              { return vaccine; }
  int get_current_dose(void)              { return current_dose; }
  
  // Modifiers
  void set_vaccination_day(int day) { 
    assert(day >= 0);
    if(vaccination_day ==-1){
      vaccination_day = day; 
    }
    else{
      //This is an error, but it will not stop a run, only pring a Warning.
      cout << "\nWARNING! Vaccination Status, setting vaccine day of someone who has already been vaccinated";
    }
  }
  
  void set_vaccine_effective_day(void) {
    assert(vaccination_day > -1);
    int vaccination_delay = vaccine
    
      
 private:
  int vaccination_day;             // On which day did you get the vaccine
  int vaccination_effective_day;   // On which day is the vaccine effective
  Vaccine* vaccine;                // Which vaccine did you take
  int current_dose;
};

#endif

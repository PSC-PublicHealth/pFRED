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

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "Random.h"
#include "Vaccine.h"
#include "VaccineDose.h"

class Vaccine_Status {
 public:
  //Creation Operations
  Vaccine_Status(void){
    vaccination_day           = -1;
    vaccination_effective_day = -1;
    current_dose              =0;
    days_to_next_dose         = -1;
  }
  
  Vaccine_Status(int day, Vaccine* vacc, int age){
    assert(day >=0);
    
    vaccine = vacc;
    vaccination_day       = day;
    double efficacy       = vacc->get_dose(0)->get_efficacy(age);
    double efficacy_delay = vacc->get_dose(0)->get_efficacy_delay(age);
    
    vaccination_effective_day = -1;
    if(RANDOM() < efficacy){
      vaccination_effective_day = day+efficacy_delay;
    }
    current_dose =0;
    days_to_next_dose = -1;
    if(vacc->get_number_doses() > 1){
      days_to_next_dose = day + vacc->get_dose(0)->get_days_between_doses();
    }
  }
  
  
  // Access Members
  int get_vaccination_day(void)           { return vaccination_day; }
  int get_vaccination_effective_day(void) { return vaccination_effective_day; }
  Vaccine* get_vaccine(void)              { return vaccine; }
  int get_current_dose(void)              { return current_dose; }
  int get_days_to_next_dose(void)         { return days_to_next_dose; }
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
 
  //Utility Functions
  void print(void){
    cout << "\nVaccine Status";
}
      
 private:
  int vaccination_day;             // On which day did you get the vaccine
  int vaccination_effective_day;   // On which day is the vaccine effective
  Vaccine* vaccine;                // Which vaccine did you take
  int current_dose;
  int days_to_next_dose;
};

#endif

/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: VaccineStatus.cc
//
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "Random.h"
#include "Vaccine_Health.h"
#include "Vaccine.h"
#include "Vaccine_Dose.h"
#include "Health.h"
#include "Person.h"
#include "Global.h"

Vaccine_Health::Vaccine_Health(int _vaccination_day, Vaccine* _vaccine, int _age, Health* _health){
  
  vaccine               = _vaccine;
  vaccination_day       = _vaccination_day;
  health                = _health;
  double efficacy       = vaccine->get_dose(0)->get_efficacy(_age);
  double efficacy_delay = vaccine->get_dose(0)->get_efficacy_delay(_age);
  
  vaccination_effective_day = -1;
  if(RANDOM() < efficacy)
    vaccination_effective_day = vaccination_day + efficacy_delay;
  current_dose =0;
  days_to_next_dose = -1;
  if(vaccine->get_number_doses() > 1){
    days_to_next_dose = vaccination_day + vaccine->get_dose(0)->get_days_between_doses();
  }
}

void Vaccine_Health::print() const {
  
  // Need to make work :)
  cout << "\nVaccine_Status";
}

void Vaccine_Health::printTrace() const {
  fprintf(VaccineTracefp," %2d %2d %2d",vaccination_day,is_effective(),current_dose);
  fflush(Tracefp);
}

void Vaccine_Health::update(int day, int age){
  // First check for immunity 
  if (is_effective() && day >= vaccination_effective_day) {
    // Going out to Person, so that behavior can be accessed
    Strain* s = health->get_self()->get_population()->get_strain(0);
    health->get_self()->become_immune(s);
  }
  
  // Next check on dose
  // Even immunized people get another dose
  // int next_dose_day = vaccination_day + days_to_next_dose;
  if (current_dose < vaccine->get_number_doses()) {
    if (day >= days_to_next_dose && current_dose) {
      current_dose++;
      days_to_next_dose = vaccine->get_dose(current_dose)->get_days_between_doses();
      if (!is_effective()) {                     // If the first dose was not effective
        double efficacy = vaccine->get_dose(current_dose)->get_efficacy(age);
        double efficacy_delay = vaccine->get_dose(0)->get_efficacy_delay(age);
        if (RANDOM() < efficacy)
          vaccination_effective_day = day + efficacy_delay;        
      }
    }
  }
}


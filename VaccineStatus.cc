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
#include "VaccineStatus.h"
#include "Vaccine.h"
#include "VaccineDose.h"
#include "Health.h"
#include "Global.h"

Vaccine_Status::Vaccine_Status(int day, Vaccine* vacc, int age, Health* h){
  assert(day >=-1);
  
  vaccine = vacc;
  vaccination_day       = day;
  health = h;
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

void Vaccine_Status::print(void){

  // Need to make work :)
  cout << "\nVaccine_Status";
}

void Vaccine_Status::printTrace(void){
  fprintf(VaccineTracefp," %2d %2d %2d",vaccination_day,is_effective(),current_dose);
  fflush(Tracefp);
}

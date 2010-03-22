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
  
  vaccine = vacc;
  vaccination_day       = day;
  health = h;
  double efficacy       = vaccine->get_dose(0)->get_efficacy(age);
  double efficacy_delay = vaccine->get_dose(0)->get_efficacy_delay(age);
  
  vaccination_effective_day = -1;
  if(RANDOM()*100. < efficacy){
    vaccination_effective_day = day+efficacy_delay;
  }
  current_dose =0;
  days_to_next_dose = -1;
  if(vacc->get_number_doses() > 1){
    days_to_next_dose = day + vaccine->get_dose(0)->get_days_between_doses();
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

void Vaccine_Status::update(int day, int age){
  // First check for immunity 
  if(is_effective()){
    if(day >= vaccination_effective_day){
      health->immunize(0); // Only works with one strain
      
    }
  }
  
  // Next check on dose
  // Even immunized people get another dose
  //int next_dose_day = vaccination_day + days_to_next_dose;
  if(current_dose < vaccine->get_number_doses()){
    if(day >= days_to_next_dose && current_dose){
      current_dose++;
      days_to_next_dose = vaccine->get_dose(current_dose)->get_days_between_doses();
      if(!is_effective()){                     // If the first dose was not effective
	double efficacy = vaccine->get_dose(current_dose)->get_efficacy(age);
	double efficacy_delay = vaccine->get_dose(0)->get_efficacy_delay(age);
	if(RANDOM()*100. < efficacy){
	  vaccination_effective_day = day + efficacy_delay;
	}
	
      }
    }
  }
}
      

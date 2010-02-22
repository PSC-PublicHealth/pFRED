/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Vaccine_Manager.h
//

#ifndef _FRED_VACCINE_MANAGER_H
#define _FRED_VACCINE_MANAGER_H

#define VACC_NO_PRIORITY  0
#define VACC_AGE_PRIORITY 1

#include <vector>
#include "Manager.h"

using namespace std;

class Manager;
class Population;
class Vaccines;
class Person;
class Policy;



class Vaccine_Manager: public Manager {
  Vaccines *Vaccs;
  vector < Person * > Priority_Queue;
  vector < Person * > Queue;
  vector < Policy * > Queue_Policies;
  
  int current_priority_queue_position;
  int current_reg_queue_position;

  //Parameters from Input
  int do_vacc;
  int vaccine_compliance;
  int vaccine_priority_scheme;
  int vaccine_priority_only;

  int vaccine_priority_age_low;
  int vaccine_priority_age_high;

  int vaccination_capacity;            // How many people can be vaccinated a day

 public:
  Vaccine_Manager(Population *P);
  
  //Parameters
  int do_vaccination(void){ return do_vacc; }
  Vaccines* get_vaccines(void) { return Vaccs; }
  vector < Person * > *get_priority_queue(void) { return &Priority_Queue;}
  vector < Person * > *get_queue(void) { return &Queue;}
  int get_number_in_priority_queue(void) { return Priority_Queue.size(); }
  int get_number_in_reg_queue(void) { return Queue.size(); }
 
  // Let's do stuff 
  void Fill_Queues(void);
  int Vaccinate(void);
  
  
  //Get Paramters
  int get_vaccine_compliance(void){return vaccine_compliance;}
  int get_vaccine_priority_age_low(void){return vaccine_priority_age_low;}
  int get_vaccine_priority_age_high(void){return vaccine_priority_age_high;}
  
  void update(int day);
};


#endif
 
  

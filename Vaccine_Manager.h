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
  //Vaccine_Manager handles a stock of vaccines
 public:
  Vaccine_Manager(Population *_pop);
  
  //Parameters Access
  bool do_vaccination(void)                     const { return do_vacc; }
  Vaccines* get_vaccines(void)                  const { return vaccine_package; }
  vector < Person* > get_priority_queue(void)   const { return priority_queue;}
  vector < Person* > get_queue(void)            const { return queue;}
  int get_number_in_priority_queue(void)        const { return priority_queue.size(); }
  int get_number_in_reg_queue(void)             const { return queue.size(); }
 
  // Vaccination Specific Procedures
  void fill_queues(void);
  void vaccinate(int day);
  
  //Paramters Access Members
  int get_vaccine_compliance(void)        const {return vaccine_compliance;}
  int get_vaccine_priority_age_low(void)  const {return vaccine_priority_age_low;}
  int get_vaccine_priority_age_high(void) const {return vaccine_priority_age_high;}
  
  // Utility Members
  void update(int day);
  void reset(void);
  void print(void);

 private:
  Vaccines* vaccine_package;              //Pointer to the vaccines that this manager oversees
  vector < Person* > priority_queue;      //Queue for the priority agents
  vector < Person* > queue;               //Queue for everyone else
  
  //Parameters from Input 
  bool   do_vacc;                         //Is Vaccination being performed
  double vaccine_compliance;              //Global compliance parameter
  int    vaccine_priority_scheme;         //Priority policy to use (see defines above)
  bool   vaccine_priority_only;           //True - Vaccinate only the priority
  
  int vaccine_priority_age_low;           //Age specific priority
  int vaccine_priority_age_high;
  
  int vaccination_capacity;               // How many people can be vaccinated a day
  
};


#endif
 
  

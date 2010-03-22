/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Manager.h
//

#ifndef _FRED_AV_MANAGER_H
#define _FRED_AV_MANAGER_H

#include "Antivirals.h"

class Manager;
class Population;
class Person;
class Policy; 

class AV_Manager: public Manager {
 public:
  AV_Manager(Population *P);

  //Parameters
  int do_antivirals(void) { return do_av;}
  int get_percent_symptomatics_given(){return percent_symptomatics_given;}
  int get_overall_start_day() { return overall_start_day;}
  Antiviral* get_current_av() { return current_av; }
    
  //Paramters
  Antivirals* get_antivirals(void) { return AVs; }
  int get_num_antivirals(void) { return AVs->get_number_antivirals();}

  // Manager Functions
  void disseminate(int day);
  
  // Utility Functions
  void update(int day);
  void reset(void);
  void print(void);

 private:
  int do_av;
  Antivirals* AVs;     // The AV manager needs to know how much AV is available
  // Parameters
  int percent_symptomatics_given;
  int overall_start_day;
  void set_policies(void);
  
  Antiviral* current_av;
};

#endif

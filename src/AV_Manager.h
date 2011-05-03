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
#include "Manager.h"

#define AV_POLICY_PERCENT_SYMPT 0
#define AV_POLICY_GIVE_EVERYONE 1

class Population;
class Person;
class Policy; 

class AV_Manager: public Manager {
public:
  AV_Manager(Population *_pop);
  
  //Parameters
  bool do_antivirals()             const { return do_av; }
  int get_overall_start_day()      const { return overall_start_day; }
  Antiviral* get_current_av()      const { return current_av; }
  
  //Paramters
  Antivirals* get_antivirals()     const { return av_package; }
  int get_num_antivirals()         const { return av_package->get_number_antivirals(); }
  bool get_are_policies_set()      const { return are_policies_set; }
  
  // Manager Functions
  void disseminate(int day);      // push avs to agents, needed for prophylaxis
  
  // Utility Functions
  void update(int day);
  void reset();
  void print();
  
private:
  bool do_av;                      //Whether or not antivirals are being disseminated
  Antivirals* av_package;          //The package of avs available to this manager
  // Parameters 
  int overall_start_day;           //Day to start the av procedure
  void set_policies();             //member to set the policy of all of the avs
  bool are_policies_set;         //Ensure that the policies for AVs have been set.
  
  Antiviral* current_av;           //NEED TO ELIMINATE, HIDDEN to IMPLEMENTATION
};

#endif

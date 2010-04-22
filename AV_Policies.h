/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Decision.h
//

#ifndef _FRED_AV_POLICIES_H
#define _FRED_AV_POLICIES_H

#include <iostream>
#include <string>
#include "Policy.h"

class Decision;
//class Policy;
class Person;
class Antivirals;
class AV_Manager;
class Manager;

using namespace std;

class AV_Policy_Distribute_To_Symptomatics: public Policy {
  AV_Manager* av_manager;
  
 public:
  AV_Policy_Distribute_To_Symptomatics(void);
  AV_Policy_Distribute_To_Symptomatics(AV_Manager* avm);
  
  virtual int choose(Person* person, int strain, int current_day);
};

class AV_Policy_Distribute_To_Everyone: public Policy {
  AV_Manager* av_manager;  
 public:
  AV_Policy_Distribute_To_Everyone(void);
  AV_Policy_Distribute_To_Everyone(AV_Manager* avm);
  
  virtual int choose(Person* person, int strain, int current_day);
};
#endif

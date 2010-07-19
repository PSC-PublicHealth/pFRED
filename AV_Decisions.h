/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: AV_Decision.h
//

#ifndef _FRED_AV_DECISIONS_H
#define _FRED_AV_DECISIONS_H

#include "AV_Policies.h"
#include "Decision.h"

class Policy;

//  AV_Policy_Allow_Only_One
//  This policy defines that each agent can only have one AV throughout
//  the course of the simulation

class AV_Decision_Allow_Only_One: public Decision {
  
public: 
  AV_Decision_Allow_Only_One(Policy * p);
  AV_Decision_Allow_Only_One();
  int evaluate(Person* person, int strain, int current_day);
};

// AV_Decision_Give_One_Chance
// This policy defines that each agent only has one chance to get AV,
// i.e. only check once.

class AV_Decision_Give_One_Chance: public Decision {
public:
  AV_Decision_Give_One_Chance(Policy *p);
  AV_Decision_Give_One_Chance();
  int evaluate(Person* person, int strain, int current_day);
};

class AV_Decision_Give_to_Sympt: public Decision {
public:
  AV_Decision_Give_to_Sympt(Policy *p);
  AV_Decision_Give_to_Sympt();
  int evaluate(Person* person, int strain, int current_day);
};

class AV_Decision_Begin_AV_On_Day: public Decision {
public:
  AV_Decision_Begin_AV_On_Day(Policy *p);
  AV_Decision_Begin_AV_On_Day();
  int evaluate(Person* person, int strain, int current_day);
};

#endif

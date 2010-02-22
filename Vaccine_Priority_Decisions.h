/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/
//
//
// File: AV_Decision.h
//

#ifndef _FRED_VACCINE_PRIORITY_DECISIONS_H
#define _FRED_VACCINE_PRIORITY_DECISIONS_H


#include "Decision.h"

class Policy;

// Vaccine_Decision_Policy_Specific_Age

class Vaccine_Priority_Decision_Specific_Age: public Decision {
 public:
  Vaccine_Priority_Decision_Specific_Age(Policy* p);
  Vaccine_Priority_Decision_Specific_Age();
  int evaluate(void);
};

class Vaccine_Priority_Decision_No_Priority: public Decision {
 public:
  Vaccine_Priority_Decision_No_Priority(Policy *p);
  Vaccine_Priority_Decision_No_Priority();
  int evaluate(void);
};

#endif

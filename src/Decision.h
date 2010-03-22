/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Decision.h
//

#ifndef _FRED_DECISION_H
#define _FRED_DECISION_H

#include <iostream>
#include <string>
#include <list>

class Policy;

using namespace std;

class Decision{

 protected:
  string Name;
  string Type; 
  Policy *policy;  // This is the policy that the decision belongs to

 public:
  Decision(void){}
  Decision(Policy *p);
  
  string get_name(void) { return Name; }
  string get_type(void) { return Type; }
  
  virtual int evaluate(void) = 0;  // There must be this funtion of all policies that 
                                   // This represents the rule in which a manager makes a decision
                                   // It can return a 0/1 for yes no decisions
                                   // it can also return a int for a decision to deliver something
};
#endif

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
class Person;

using namespace std;

class Decision{

 protected:
  string name;
  string type; 
  Policy *policy;  // This is the policy that the decision belongs to

 public:
  Decision(void);
  Decision(Policy *p);
  ~Decision();
  
  string get_name(void) const { return name; }
  string get_type(void) const { return type; }
  
  virtual int evaluate(Person* person, int strain, int current_day) = 0;  
};
#endif

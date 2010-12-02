/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Policy.h
//

// A Policy is a class that is accessed by the manager to decide something.
//   Will be used for mitigation strategies.

#ifndef _FRED_POLICY_H
#define _FRED_POLICY_H

#include <iostream>
#include <string>
#include <vector>

class Manager;
class Decision;
class Person;

using namespace std;

class Policy{
  
public:
  Policy();
  ~Policy();
  
  Policy(Manager* mgr);
  
  virtual int choose(Person* person, int disease, int current_day);         
  // decision will return -1 if the decision is no
  // or the integer result of the policies in the decision   
  
  Manager* get_manager() const { return manager; }
  void print() const;
  void reset();
  
protected:
  vector < Decision * > decision_list;
  string Name;
  Manager* manager;
  
};

#endif

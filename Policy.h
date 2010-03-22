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

using namespace std;

class Policy{
  
 protected:
  vector < Decision * > Decision_list;
  string Name;
  Manager* manager;

 public:
  Policy(void) { }
  Policy(Manager* mgr);
  
  virtual int choose(void) = 0;          // decision will return -1 if the decision is no
                                        // or the integer result of the policies in the decision   
  
  Manager* get_manager(void){ return manager; }
  void print(void);
  void reset(void);
};

#endif

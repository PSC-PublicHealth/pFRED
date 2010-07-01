/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Manager.h
//

#ifndef _FRED_MANAGER_H
#define _FRED_MANAGER_H

#include <iostream>
#include <vector>

class Policy;
class Population;
class Person;
class Person;
class Decision;

using namespace std;

class Manager{
  // Manager is an abstract class that is the embodiment of a mitigation manager.
  // The Manager: 
  //    1. Handles a stock of mitigation supplies
  //    2. Holds the policy for doling out a mitigation strategy
 protected:
  vector <Policy * > policies;   // vector to hold the policies this manager can apply
  vector < int > results;        // DEPRICATE holds the results of the policies
  Population *pop;               // Population in which this manager is tied to
  int current_policy;            // The current policy this manager is using

 public:
  Manager(void);
  Manager(Population *_pop);
  ~Manager(void);
  virtual int poll_manager(Person* p, int strain, int day); //member to allow someone to see if they fit the current policy

  // Parameters
  Population* get_population(void) const { return pop;}
  int get_current_policy(void)     const {return current_policy; } 

  //Utility Members
  virtual void update(int day) { };
  virtual void reset(void) { };
  virtual void print(void) { };
  
};
  
#endif

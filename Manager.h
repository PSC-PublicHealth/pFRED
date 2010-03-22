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
class Decision;

using namespace std;
class Manager{
 protected:
  vector <Policy * > Policies;
  vector < int > Results;
  Population *Pop;
  Person* current_person;
  int current_strain;
  int current_policy;
  int current_day;

 public:
  Manager(void) {  }
  Manager(Population *P);
  virtual int poll_manager(Person* p, int strain, int day);  // stain is here... and that sucks.

  // Parameters
  Population* get_population(void){ return Pop;}
  Person* get_current_person(void) { return current_person; }
  int get_current_policy(void){return current_policy; }
  int get_current_strain(void){return current_strain; }
  int get_current_day(void){return current_day;}
  
  virtual void update(int day) { };
  virtual void reset(void) { };
  virtual void print(void) { };
  
};
  
#endif

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Manager.cpp
//

#include "Manager.h"
#include "Policy.h"
#include <iostream>
#include <list>
#include <vector>

using namespace std;

Manager::Manager(Population *P){
  Pop = P;
  //current_person = Pop-> // need to figure out how to set this
  current_strain = 0;
  current_policy = 0;
  current_day = 0;
}

int Manager::poll_manager(Person* p, int strain, int day){
  current_person = p;
  current_strain = strain;
  current_day = day;
  int result = Policies[current_policy]->choose();
  return result;
}

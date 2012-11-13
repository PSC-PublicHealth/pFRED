/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Manager.cpp
//

#include <iostream>
#include <list>
#include <vector>

#include "Manager.h"
#include "Policy.h"
#include "Person.h"

using namespace std;

Manager::Manager() {
  pop = NULL;
  current_policy = -1;
}

Manager::~Manager() {
  for(unsigned int i=0; i < policies.size(); i++) delete policies[i];
  policies.clear();
  results.clear();
}

Manager::Manager(Population *P){
  pop = P;
  current_policy = 0;
}

int Manager::poll_manager(Person* p, int disease, int day){
  int result = policies[current_policy]->choose(p,disease,day);
  return result;
}

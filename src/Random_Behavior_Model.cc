/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Random_Behavior_Model.cc
//

#include "Random_Behavior_Model.h"
#include "Person.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"


#include <stdio.h>
#include <iostream>

using namespace std;

double Random_Behavior_Model::prob_stay_home = 0.0;
bool Random_Behavior_Model::initialized = false;

Random_Behavior_Model::Random_Behavior_Model(Person *p) {
  self = p;
  have_decided = false;
  staying_home = false;

  if (Random_Behavior_Model::initialized == false) {
    Params::get_param((char *) "prob_stay_home", &Random_Behavior_Model::prob_stay_home);
    Random_Behavior_Model::initialized = true;
  }
}

bool Random_Behavior_Model::is_staying_home(int day) {
  if (have_decided == false) {
    staying_home = (RANDOM() < Random_Behavior_Model::prob_stay_home);
    have_decided = true;
  }
  return staying_home;
}



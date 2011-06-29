/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.cc
//

#include "Behavior.h"
#include "Random_Behavior_Model.h"
#include "Health_Belief_Model.h"
#include "Person.h"
#include "Global.h"
#include "Utils.h"

#include <string.h>

Behavior::Behavior(Person *p) {
  if (strcmp(Behavior_model_type, "random") == 0) {
    model = new Random_Behavior_Model(p);
  }
  else if (strcmp(Behavior_model_type, "HBM") == 0) {
    model = new Health_Belief_Model(p);
  }
  else {
    Utils::fred_abort("Help! Unrecognized behavior model: |%s|\n", Behavior_model_type); 
  }
}


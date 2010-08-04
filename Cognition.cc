/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cognition.cc
//

#include "Cognition.h"
#include "Random_Cognitive_Model.h"
#include "Health_Belief_Model.h"
#include "Person.h"
#include "Global.h"

Cognition::Cognition(Person *p) {
  if (strcmp(Cognitive_model_type, "random") == 0) {
    model = new Random_Cognitive_Model(p);
  }
  else if (strcmp(Cognitive_model_type, "HBM") == 0) {
    model = new Health_Belief_Model(p);
  }
  else {
    printf("Help! Unrecognized cognitive model: |%s|\n", Cognitive_model_type);
    abort();
  }
}


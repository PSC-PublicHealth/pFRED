/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Perceptions.cc
//

#include "Perceptions.h"
#include "Person.h"

Perceptions::Perceptions(Person *p) {
  me = p;
}

void Perceptions::reset() {
  perceived_susceptibility = 0.0;
  perceived_severity = 0.0;
  perceived_benefits = 0.0;
  perceived_barriers = 0.0;
  self_efficacy = 0.0;
}

void Perceptions::update(int day) {
  // update perceived_susceptibility

  // update perceived_severity

  // update perceived_benefits

  // update perceived_barriers

  // update self_efficacy
}



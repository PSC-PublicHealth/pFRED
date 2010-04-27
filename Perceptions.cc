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
#include "Place.h"
#include "Global.h"
#include "Random.h"


Perceptions::Perceptions(Person *p) {
  self = p;
}

void Perceptions::reset() {
  perceived_susceptibility = 0.0;
  perceived_severity = 0.0;
  perceived_benefits = 0.0;
  perceived_barriers = 0.0;
  self_efficacy = 0.0;
  keep_kids_home = 0;
}

void Perceptions::update(int day) {
  Person * HoH;
  HoH = self->get_behavior()->get_HoH();
  keep_kids_home = 0;
  if (self == HoH) {
    if (Use_HBM) HBM(day);
  }
  else {
    if (self->get_age() < 18) {
      if (HoH != NULL) {
	// printf("kid %d %d   hoh %d %d  ", self->get_id(), self->get_age(), HoH->get_id(), HoH->get_age()); fflush(stdout);
	keep_kids_home = HoH->get_perceptions()->will_keep_kids_home();
	// printf("decision = %d\n", keep_kids_home); fflush(stdout);
      }
    }
  }
}

void Perceptions::HBM(int day) {
  double prob_action;
  Place * household = self->get_behavior()->get_household();


  // update perceived_susceptibility
  // get the max sympt rate 
  perceived_susceptibility = 0;
  int schools = household->get_num_schools();
  // printf("self %d %d  schools = %d\n", self->get_id(), self->get_age(), schools); fflush(stdout);
  if (schools == 0) return;
  for (int s = 0; s < schools; s++) {
    Place * sch = household->get_school(s);
    // printf("school id = %d\n", sch->get_id()); fflush(stdout);
    double sick = ((double)sch->get_symptomatic(0)) / ((double)sch->get_size()); 
    // double sick = sch->get_symptomatic(0);
    // printf("school sick = %f\n", sick); fflush(stdout);
    perceived_susceptibility += sick;
  }
  if (perceived_susceptibility > 1.0) perceived_susceptibility = 1.0;
    
  // update perceived_severity
  perceived_severity = 1.0;
    
  // update perceived_benefits
  perceived_benefits = 1.0 * household->get_children();
  // if (perceived_benefits > 1.0) perceived_benefits = 1.0;
    
  // update perceived_barriers
  perceived_barriers = 0.0;
    
  // update self_efficacy
  self_efficacy = 1.0;

  prob_action = W1*perceived_susceptibility;
  prob_action *= W2*perceived_severity;
  prob_action *= W3*perceived_benefits;
  prob_action -= HBM_threshold;

  keep_kids_home = (RANDOM() < prob_action);

  if (keep_kids_home) {
    printf("day %3d  sus %8.6f  sev %3.1f  ben %0.1f  prob %0.2f  dec %d\n",
	   day, perceived_susceptibility, perceived_severity, perceived_benefits, prob_action, keep_kids_home);
    fflush(stdout);
  }
}

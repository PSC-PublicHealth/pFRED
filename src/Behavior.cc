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
#include "Person.h"
#include "Cognition.h"
#include "Global.h"
#include "Profile.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#include "Date.h"
#include "Patch.h"

Behavior::Behavior (Person *person, Place **fav_place, int pro) {
  self = person;
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    this->favorite_place[i] = fav_place[i];
    if (fav_place[i] != NULL) {
      fav_place[i]->add_person(self);
    }
  }
  assert(get_household() != NULL);

  // get the neighbhood from the household
  favorite_place[NEIGHBORHOOD_INDEX] = favorite_place[HOUSEHOLD_INDEX]->get_patch()->get_neighborhood();
  if (get_neighborhood() == NULL) { 
    printf("person %d house %d \n", person->get_id(), get_household()->get_id());
  }
  assert(get_neighborhood() != NULL);
  favorite_place[NEIGHBORHOOD_INDEX]->add_person(self);

  profile = pro;
  schedule_updated = -1;
}

void Behavior::reset() {
  // reset the daily schedule
  schedule_updated = -1;
}

void Behavior::update(int day) {
  // we avoid computing this except as happens below
  // update_schedule(day);
}

void Behavior::update_infectious_behavior(int day) {
  int diseases = self->get_diseases();
  for (int dis = 0; dis < diseases; dis++) {
    if (self->is_infectious(dis)) {
      char status = self->get_disease_status(dis);
      if (schedule_updated < day) update_schedule(day);
      for (int i = 0; i < FAVORITE_PLACES; i++) {
	if (on_schedule[i]) {
	  favorite_place[i]->add_infectious(dis, self, status);
	}
      }
    }
  }
}

void Behavior::update_susceptible_behavior(int day) {
  int diseases = self->get_diseases();
  for (int dis = 0; dis < diseases; dis++) {
    if (self->is_susceptible(dis)) {
      for (int i = 0; i < FAVORITE_PLACES; i++) {
	if (favorite_place[i] != NULL && favorite_place[i]->is_infectious(dis)) {
	  if (schedule_updated < day) update_schedule(day);
	  if (on_schedule[i]) {
	    favorite_place[i]->add_susceptible(dis, self);
	  }
	}
      }
    }
  }
}

void Behavior::update_schedule(int day) {
  int day_of_week;
  if (schedule_updated < day) {
    day_of_week = Fred_Date->get_day_of_week(day);
    schedule_updated = day;
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      on_schedule[p] = false;
    }
		
    // decide whether to stay at home if symptomatic
    int is_symptomatic = self->is_symptomatic();
    if (is_symptomatic) {
      if (RANDOM() < Disease::get_prob_stay_home()) {
	on_schedule[HOUSEHOLD_INDEX] = true;
	return;
      }
    }
		
    // decide whether to stay home from school
    if (0 < day_of_week && day_of_week < 6 && self->get_age() < 18) {
      if (self->get_cognition()->will_keep_kids_home()) {
	on_schedule[HOUSEHOLD_INDEX] = true;
	return;
      }
    }
		
    // if not staying home or traveling, consult usual schedule
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      if (favorite_place[p] != NULL)
	on_schedule[p] = is_visited(p, profile, day_of_week);
    }

    if (on_schedule[NEIGHBORHOOD_INDEX]) {
      // pick the neighborhood to visit today
      favorite_place[NEIGHBORHOOD_INDEX] = favorite_place[HOUSEHOLD_INDEX]->get_patch()->select_neighborhood();
    }

    // visit classroom or office iff going to school or work
    on_schedule[CLASSROOM_INDEX] = on_schedule[SCHOOL_INDEX];
    on_schedule[OFFICE_INDEX] = on_schedule[WORKPLACE_INDEX];
		
    if (Verbose > 2) {
      printf("update_schedule on day %d\n", day);
      print_schedule();
      fflush(stdout);
    }
  }
}

void Behavior::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d  ", self->get_id());
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (on_schedule[p])
      fprintf(Statusfp, "%d ", favorite_place[p]->get_id());
    else {
      if (favorite_place[p] == NULL) {
	fprintf(Statusfp, "-1 ");
      }
      else {
	fprintf(Statusfp, "-%d ", favorite_place[p]->get_id());
      }
    }
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}

bool Behavior::acceptance_of_vaccine() {
  return self->get_cognition()->will_accept_vaccine(0);
}

bool Behavior::acceptance_of_another_vaccine_dose() {
  return self->get_cognition()->will_accept_another_vaccine_dose(0);
}


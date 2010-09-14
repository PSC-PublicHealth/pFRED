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
#include "Community.h"
#include "Random.h"
#include "Strain.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#include "Household.h"

#define DAYS_PER_WEEK 7

Behavior::Behavior (Person *person, Place **fav_place, int pro) {
  self = person;
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    this->favorite_place[i] = fav_place[i];
    if (fav_place[i] != NULL) {
      fav_place[i]->add_person(self);
    }
  }
  assert(get_household() != NULL);
  profile = pro;
  schedule_updated = -1;
}

void Behavior::reset() {
  // add myself to the susceptible lists at my favorite places
  int strains = self->get_population()->get_strains();
  for (int strain = 0; strain < strains; strain++) {
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      if (favorite_place[p] == NULL) continue;
      favorite_place[p]->add_susceptible(strain, self);
    }
    community->add_susceptible(strain, self);
  }
	
  // reset the daily schedule
  schedule_updated = -1;
}

void Behavior::update(int day) {
}

bool Behavior::is_on_schedule(int day, int loc, char loctype) {
  int p = 0; 
  if (schedule_updated < day) 
    update_schedule(day);
  switch (loctype) {
  case HOUSEHOLD: p = HOUSEHOLD_INDEX; break;
  case NEIGHBORHOOD: p = NEIGHBORHOOD_INDEX; break;
  case COMMUNITY: p = NEIGHBORHOOD_INDEX; break;
  case SCHOOL: p = SCHOOL_INDEX; break;
  case CLASSROOM: p = CLASSROOM_INDEX; break;
  case WORKPLACE: p = WORKPLACE_INDEX; break;
  case OFFICE: p = OFFICE_INDEX; break;
  default: assert(strcmp("Bad loctype", "") == 0);
  }
  assert(loctype == COMMUNITY || (favorite_place[p]->get_id() == loc));
  return on_schedule[p];
}

void Behavior::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d  ", self->get_id());
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (on_schedule[p])
      fprintf(Statusfp, "%d ", favorite_place[p]->get_id());
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}

void Behavior::update_schedule(int day) {
  int day_of_week;
  if (schedule_updated < day) {
    day_of_week = (day + Start_day_of_week) % DAYS_PER_WEEK;
    schedule_updated = day;
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      on_schedule[p] = false;
    }
		
    // decide whether to stay at home if symptomatic
    int is_symptomatic = self->is_symptomatic();
    if (is_symptomatic) {
      if (RANDOM() < Strain::get_prob_stay_home()) {
	on_schedule[0] = true;
	return;
      }
    }
		
    // decide whether to stay home from school
    if (0 < day_of_week && day_of_week < 6 && self->get_age() < 18) {
      if (self->get_cognition()->will_keep_kids_home()) {
	on_schedule[0] = true;
	return;
      }
    }
		
    // if not staying home or traveling, consult usual schedule
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      if (favorite_place[p] == NULL) continue;

      // visit classroom or office iff going to school or work
      if (p == CLASSROOM_INDEX || p == OFFICE_INDEX) {
	on_schedule[p] = on_schedule[p-1];
      }
      else {
	on_schedule[p] = is_visited(p, profile, day_of_week);
      }
    }
		
    if (Verbose > 2) {
      printf("update_schedule on day %d\n", day);
      print_schedule();
      fflush(stdout);
    }
  }
}

void Behavior::get_schedule(int *n, Place **sched) {
  *n = 0;
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (on_schedule[p]) {
      sched[(*n)++] = favorite_place[p];
    }
  }
}


void Behavior::become_susceptible(int strain) {
  // add me to susceptible list at my favorite places
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL)
      favorite_place[p]->add_susceptible(strain, self);
  }
}

void Behavior::become_exposed(int strain) {
  // remove me from susceptible list at my favorite places
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL)
      favorite_place[p]->delete_susceptible(strain, self);
  }
}

void Behavior::become_infectious(int strain) {
  // add me to infectious list at my favorite places
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL) {
      if (Test) {
	if (self->get_exposure_date(strain) == 0) {
	  favorite_place[p]->add_infectious(strain, self);
	}
      } else {
	favorite_place[p]->add_infectious(strain, self);
      }
    }
  }
  community->add_infectious(strain, self);
}

void Behavior::become_immune(int strain) {
  // remove me from the susceptible list at my favorite places
  // STB - While this is really the same as become_exposed, I thought it important to 
  //       make sure there was a differentiation
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL) 
      favorite_place[p]->delete_susceptible(strain, self);
  }
}

void Behavior::recover(int strain) {
  // remove me from infectious list at my favorite places
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL) {
      if (Test) {
	if (self->get_exposure_date(strain) == 0) {
	  favorite_place[p]->delete_infectious(strain, self);
	}
      } else {
	favorite_place[p]->delete_infectious(strain, self);
      }
    }
  }
  community->delete_infectious(strain, self);
}


int Behavior::compliance_to_vaccination(){
  return self->get_cognition()->will_accept_vaccine(0);
}


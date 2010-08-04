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
#include "Strain.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#include "Household.h"

#define DAYS_PER_WEEK 7

Behavior::Behavior (Person *person, Place **fav_place, int pro) {
  self = person;
  favorite_places = 6;
  this->favorite_place = new Place * [favorite_places];
  for (int i = 0; i < favorite_places; i++) {
    this->favorite_place[i] = fav_place[i];
    if (fav_place[i] != NULL) fav_place[i]->add_person(self);
  }
  if (get_household() == NULL) {
    printf("Help! person %d has no home.\n", person->get_id()); abort();
  }
  // printf("home = %d\n", get_household()->get_id()); fflush(stdout);
  profile = pro;
  schedule = new Place * [favorite_places];
  on_schedule = new char [favorite_places];
  schedule_updated = -1;
  scheduled_places = 0;
}

void Behavior::reset() {
  // add myself to the susceptible lists at my favorite places
  int strains = self->get_population()->get_strains();
  for (int strain = 0; strain < strains; strain++) {
    for (int p = 0; p < favorite_places; p++) {
      if (favorite_place[p] == NULL) continue;
      favorite_place[p]->add_susceptible(strain, self);
    }
  }
	
  // reset the daily schedule
  schedule_updated = -1;
  scheduled_places = 0;
}

void Behavior::update(int day) {
}

void Behavior::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d  ", self->get_id());
  fprintf(Statusfp, "scheduled places %d\n", scheduled_places);
  for (int j = 0; j < scheduled_places; j++) {
    fprintf(Statusfp, "%d ", schedule[j]->get_id());
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}

int Behavior::is_on_schedule(int day, int loc) {
  if (schedule_updated < day) 
    update_schedule(day);
  int p = 0; 
  while (p < scheduled_places && (schedule[p]->get_id() != loc)) p++;
  return (p < scheduled_places);
}

void Behavior::update_schedule(int day) {
  int day_of_week;
  if (schedule_updated < day) {
    schedule_updated = day;
    scheduled_places = 0;
    for (int p = 0; p < favorite_places; p++) {
      on_schedule[p] = 0;
    }
    day_of_week = (day+Start_day) % DAYS_PER_WEEK;
		
    // decide whether to stay at home if symptomatic
    int is_symptomatic = self->is_symptomatic();
    if (is_symptomatic) {
      if (self->get_cognition()->will_stay_home_if_sick()) {
	schedule[0] = get_household();
	scheduled_places = 1;
	on_schedule[0] = 1;
      }
    }
		
    // decide whether to stay home from school
    if (0 < day_of_week && day_of_week < 6) {
      if (scheduled_places == 0 && self->get_age() < 18) {
	if (self->get_cognition()->will_keep_kids_home()) {
	  schedule[0] = get_household();
	  scheduled_places = 1;
	  on_schedule[0] = 1;
	  printf("day %d age %d staying home\n",day,self->get_age());
	  fflush(stdout);
	}
      }
    }
		
    // if not staying home or traveling, consult usual schedule
    if (scheduled_places == 0) {
      for (int p = 0; p < favorite_places; p++) {
	// visit classroom or office iff going to school or work
	if (p == 3 || p == 5) {
	  if (on_schedule[p-1]) {
	    on_schedule[p] = 1;
	    schedule[scheduled_places++] = favorite_place[p];
	  }
	} else if (favorite_place[p] != NULL &&
                   is_visited(p, profile, day_of_week)) {
	  on_schedule[p] = 1;
	  schedule[scheduled_places++] = favorite_place[p];
	}
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
  *n = scheduled_places;
  for (int i = 0; i < scheduled_places; i++)
    sched[i] = schedule[i];
}

void Behavior::become_susceptible(int strain) {
  // add me to susceptible list at my favorite places
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    favorite_place[p]->add_susceptible(strain, self);
  }
}

void Behavior::become_exposed(int strain) {
  // remove me from susceptible list at my favorite places
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    favorite_place[p]->delete_susceptible(strain, self);
  }
}

void Behavior::become_infectious(int strain) {
  // add me to infectious list at my favorite places
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    if (Test) {
      if (self->get_exposure_date(strain) == 0) {
	favorite_place[p]->add_infectious(strain, self);
      }
    } else {
      favorite_place[p]->add_infectious(strain, self);
    }
  }
}

void Behavior::become_immune(int strain) {
  // remove me from the susceptible list at my favorite places
  // STB - While this is really the same as become_exposed, I thought it important to 
  //       make sure there was a differentiation
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    favorite_place[p]->delete_susceptible(strain, self);
  }
}

void Behavior::recover(int strain) {
  // remove me from infectious list at my favorite places
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    if (Test) {
      if (self->get_exposure_date(strain) == 0) {
	favorite_place[p]->delete_infectious(strain, self);
      }
    } else {
      favorite_place[p]->delete_infectious(strain, self);
    }
  }
}

int Behavior::compliance_to_vaccination(){
  // Get the populations compliance
  double compliance = self->get_population()->get_vaccine_manager()->get_vaccine_compliance();
  if(RANDOM() < compliance){
    return 1;
  }
  return 0;
}


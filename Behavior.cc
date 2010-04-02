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
#include "Global.h"
#include "Profile.h"
#include "Place.h"
#include "Random.h"
#include "Strain.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#define DAYS_PER_WEEK 7

Behavior::Behavior (Person *person, Place *h, Place *n, Place *s, Place *c,
		    Place *w, Place *off, int pro) {
  self = person;
  favorite_places = 6;
  favorite_place = new Place * [favorite_places];
  favorite_place[0] = h;
  favorite_place[1] = n;
  favorite_place[2] = s;
  favorite_place[3] = c;
  favorite_place[4] = w;
  favorite_place[5] = off;
  profile = pro;
  schedule = new Place * [favorite_places];
  on_schedule = new char [favorite_places];
  schedule_updated = -1;
  scheduled_places = 0;
}

void Behavior::reset() {
  // add myself to the susceptible lists at my favorite places
  int strains = Pop.get_strains();
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
  fprintf(Statusfp, "Schedule for person %d\n", self->get_id());
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

    // probably stay at home if symptomatic
    int is_symptomatic = self->is_symptomatic();
    if (is_symptomatic) {
      if (RANDOM() < Strain::get_prob_stay_home()) {
	scheduled_places = 1;
	schedule[0] = favorite_place[0];
	on_schedule[0] = 1;
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
	}
	else if (favorite_place[p] != NULL &&
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

void Behavior::get_schedule(int *n, int *sched) {
  *n = scheduled_places;
  for (int i = 0; i < scheduled_places; i++)
    sched[i] = schedule[i]->get_id();
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
    if (Test == 0 || self->get_infectious_date(strain) == 0) {
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
    if (Test == 0 || self->get_exposure_date(strain) == 0) {
      favorite_place[p]->delete_infectious(strain, self);
    }
  }
}

int Behavior::compliance_to_vaccination(void){
  // Get the populations compliance
  double compliance = self->get_population()->get_vaccine_manager()->get_vaccine_compliance();
  if(RANDOM()*100. < compliance){
    return 1;
  }
  return 0;
}

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.cpp
//

#include "Person.hpp"
#include "Global.hpp"
#include "Profile.hpp"
#include "Place.hpp"
#include "Random.hpp"
#include "Strain.hpp"
#define DAYS_PER_WEEK 7

Person::Behavior::Behavior (Person *person, Place *h, Place *n, Place *s, Place *c,
		    Place *w, Place *off, int pro) {
  me = person;
  id = me->get_id();
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

void Person::Behavior::reset() {
  // add me to the susceptible lists at my favorite places
  int strains = Pop.get_strains();
  for (int strain = 0; strain < strains; strain++) {
    for (int p = 0; p < favorite_places; p++) {
      if (favorite_place[p] == NULL) continue;
      favorite_place[p]->add_susceptible(strain, me);
    }
  }

  // reset the daily schedule
  schedule_updated = -1;
  scheduled_places = 0;
}

void Person::Behavior::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d\n", id);
  for (int j = 0; j < scheduled_places; j++) {
    fprintf(Statusfp, "%d ", schedule[j]->get_id());
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}
  
int Person::Behavior::is_on_schedule(int day, int loc, int is_symptomatic) {
  int p = 0;
  if (schedule_updated < day) 
    update_schedule(day, is_symptomatic);
  while (p < scheduled_places && schedule[p]->get_id() != loc) p++;
  return (p < scheduled_places);
}

void Person::Behavior::update_schedule(int day, int is_symptomatic) {
  int day_of_week;
  if (schedule_updated < day) {
    schedule_updated = day;
    scheduled_places = 0;
    for (int p = 0; p < favorite_places; p++) {
      on_schedule[p] = 0;
    }
    day_of_week = (day+Start_day) % DAYS_PER_WEEK;

    // probably stay at home if symptomatic
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

void Person::Behavior::get_schedule(int *n, int *sched) {
  *n = scheduled_places;
  for (int i = 0; i < scheduled_places; i++)
    sched[i] = schedule[i]->get_id();
}

void Person::Behavior::become_infectious(int strain, int exposure_date) {
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    favorite_place[p]->delete_susceptible(strain, me);
    if (Test == 0 || exposure_date == 0) {
      favorite_place[p]->add_infectious(strain, me);
    }
  }
}

void Person::Behavior::recover(int strain, int exposure_date) {
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == NULL) continue;
    if (Test == 0 || exposure_date == 0)
      favorite_place[p]->delete_infectious(strain, me);
  }
}


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Spread.cc
//

#include "Spread.h"
#include "Strain.h"

#include <stdio.h>
#include <new>
#include <iostream>
using namespace std;
#include "Random.h"
#include "Params.h"
#include "Person.h"
#include "Behavior.h"
#include "Global.h"
#include "Infection.h"
#include "Locations.h"
#include "Place.h"
#include "Community.h"
#include "Timestep_Map.h"

extern int V_count;

Spread::Spread(Strain *str, Timestep_Map* _primary_cases_map) {
  strain = str;
  id = strain->get_id();
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  new_cases = new int [Days];
}

Spread::~Spread() {
  delete primary_cases_map;
}

void Spread::reset() {
  infected.clear();
  infectious.clear();
  attack_rate = 0.0;
  N = strain->get_population()->get_pop_size();
  total_incidents = 0;
  total_clinical_incidents = 0;
  r_index = V_count = S_count = C_count = c_count = 0;
  E_count = I_count = i_count = R_count = r_count = M_count = 0;
}

void Spread::update_stats(int day) {
  clinical_incidents = c_count;
  incident_infections = C_count;
  vaccine_acceptance = V_count;
  V_count = 0;
  new_cases[day] = incident_infections;
  total_incidents += incident_infections;
  total_clinical_incidents += clinical_incidents;
  attack_rate = (100.0*total_incidents)/N;
  clinical_attack_rate = (100.0*total_clinical_incidents)/N;

  // get reproductive rate for those infected max_days ago;
  int rday = day - strain->get_max_days();
  int rcount = 0;
  RR = 0.0;
  NR = 0;
  if (rday >= 0) {
    NR = new_cases[rday];
    for (int i = r_index; i < r_index + NR; i++) {
      rcount += infected[i]->get_infectees(id);
    }
    r_index += NR;
    if (NR)
      RR = (double)rcount / (double)NR;
  }
}

void Spread::print_stats(int day) {
  fprintf(Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	  day, id, S_count, E_count, I_count+i_count, I_count, R_count+r_count, M_count, C_count, N, attack_rate, clinical_incidents, vaccine_acceptance, RR,NR, clinical_attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	    day, id, S_count, E_count, I_count+i_count, I_count, R_count+r_count, M_count, C_count, N, attack_rate, clinical_incidents, vaccine_acceptance, RR,NR, clinical_attack_rate);
    fflush(Statusfp);
  }
  C_count = c_count = 0;
}

void Spread::update(int day) {
  set <Place *> inf_households;
  set <Place *> inf_neighborhoods;
  set <Place *> inf_classrooms;
  set <Place *> inf_schools;
  set <Place *> inf_workplaces;
  set <Place *> inf_offices;
  set<Person *>::iterator itr;
  set<Place *>::iterator it;
  Person **pop = strain->get_population()->get_pop();
  
  // See if there are changes to primary_cases_per_day from primary_cases_map
  int primary_cases_per_day = primary_cases_map->get_value_for_timestep(day);
  
  // Attempt to infect primary_cases_per_day.
  // This represents external sources of infection.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  for (int i = 0; i < primary_cases_per_day; i++) {
    int n = IRAND(0, N-1);
    if (pop[n]->get_strain_status(id) == 'S') {
      // primary infections are a special case in which
      // we don't roll the dice to see if 
      // we'll expose the person.  We just do it.
      Infection * infection = new Infection(strain, NULL, pop[n], NULL, 0);
      pop[n]->become_exposed(infection);
    }
  }
  
  // get list of infectious locations:
  for (itr = infectious.begin(); itr != infectious.end(); itr++) {
    Person * p = *itr;
    if (Verbose > 1) {
      fprintf(Statusfp, "day %d infectious person %d \n", day, p->get_id());
      fflush(Statusfp);
    }
    p->update_schedule(day);
    Place *schedule[FAVORITE_PLACES];
    int n;
    p->get_schedule(&n, schedule);
    for (int j = 0; j < n; j++) {
      Place * place = schedule[j];
      if (place && place->is_open(day) && place->should_be_open(day, id)) {
	switch (place->get_type()) {
	case HOUSEHOLD:
	  inf_households.insert(place);
	  break;

	case NEIGHBORHOOD:
	  inf_neighborhoods.insert(place);
	  break;

	case CLASSROOM:
	  inf_classrooms.insert(place);
	  break;

	case SCHOOL:
	  inf_schools.insert(place);
	  break;

	case WORKPLACE:
	  inf_workplaces.insert(place);
	  break;

	case OFFICE:
	  inf_offices.insert(place);
	  break;
	}
      }
    }
  }
  
  int infectious_places;
  infectious_places = (int) inf_households.size();
  infectious_places += (int) inf_neighborhoods.size();
  infectious_places += (int) inf_schools.size();
  infectious_places += (int) inf_classrooms.size();
  infectious_places += (int) inf_workplaces.size();
  infectious_places += (int) inf_offices.size();
  if (Verbose) {
    fprintf(Statusfp, "Number of infectious places = %d\n", infectious_places);
  }
  
  for (it = inf_classrooms.begin(); it != inf_classrooms.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_schools.begin(); it != inf_schools.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_offices.begin(); it != inf_offices.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_workplaces.begin(); it != inf_workplaces.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_neighborhoods.begin(); it != inf_neighborhoods.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_households.begin(); it != inf_households.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread strain %i in location: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }

  Commune->spread_infection_in_community(day, id);
}

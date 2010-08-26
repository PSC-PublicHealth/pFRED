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
#include "Global.h"
#include "Infection.h"
#include "Locations.h"
#include "Place.h"
#include "Timestep_Map.h"

Spread::Spread(Strain *str, Timestep_Map* _primary_cases_map) {
  strain = str;
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  new_cases = new int [Days];
}

Spread::~Spread() {
  delete primary_cases_map;
}

void Spread::reset() {
  exposed.clear();
  infectious.clear();
  attack_rate = 0.0;
  S = E = I = I_s = R = M = 0;
  total_incidents = 0;
  total_clinical_incidents = 0;
  infected.clear();
  r_index = 0;
}

void Spread::update_stats(int day) {
  int strain_id = strain->get_id();
  Person **pop = strain->get_population()->get_pop();
  int pop_size = strain->get_population()->get_pop_size();
  clinical_incidents = 0;
  incident_infections = 0;
  vaccine_acceptance = 0;
  new_cases[day] = 0;
  S = E = I = I_s = R = M = 0;
  for (int p = 0; p < pop_size; p++) {
    char status = pop[p]->get_strain_status(strain_id);
    switch ( status ) {
    case 'S' : S++; break;
    case 'E' : E++; break;
    case 'I' : I++; I_s++; break;
    case 'i' : I++; break;
    case 'R' : R++; break;
    case 'M' : M++; break;
    }

    if (pop[p]->is_new_case(day, strain_id)) {
      incident_infections++;
      infected.push_back(pop[p]);
      new_cases[day]++;
    }
    clinical_incidents += ((status=='I') && (pop[p]->get_infectious_date(strain_id) == day));
    vaccine_acceptance += pop[p]->get_behavior()->compliance_to_vaccination();
  }
  total_incidents += incident_infections;
  total_clinical_incidents += clinical_incidents;
  attack_rate = (100.0*total_incidents)/pop_size;
  clinical_attack_rate = (100.0*total_clinical_incidents)/pop_size;

  // get reproductive rate for those infected max_days ago;
  int rday = day - strain->get_max_days();;
  int rcount = 0;
  RR = 0.0;
  NR = 0;
  if (rday >= 0) {
    NR = new_cases[rday];
    for (int i = r_index; i < r_index + NR; i++) {
      rcount += infected[i]->get_infectees(strain->get_id());
    }
    r_index += NR;
    if (NR)
      RR = (double)rcount / (double)NR;
  }
}

void Spread::print_stats(int day) {
  int N = S+E+I+R+M;
  fprintf(Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	  day, strain->get_id(), S, E, I, I_s, R, M, incident_infections, N, attack_rate, clinical_incidents, vaccine_acceptance, RR,NR, clinical_attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	  day, strain->get_id(), S, E, I, I_s, R, M, incident_infections, N, attack_rate, clinical_incidents, vaccine_acceptance, RR,NR, clinical_attack_rate);
    fflush(Statusfp);
  }
}

void Spread::insert_into_exposed_list(Person * per) {
  exposed.insert(per);
}

void Spread::insert_into_infectious_list(Person * per) {
  infectious.insert(per);
}

void Spread::remove_from_exposed_list(Person * per) {
  exposed.erase(per);
}

void Spread::remove_from_infectious_list(Person * per) {
  infectious.erase(per);
}

bool Spread::is_in_exposed_list(Person *per) {
	return exposed.count(per) != 0;
}

bool Spread::is_in_infectious_list(Person *per) {
	return infectious.count(per) != 0;
}

void Spread::update(int day) {
  set <Place *> places;
  set<Person *>::iterator itr;
  set<Place *>::iterator it;
  Person **pop = strain->get_population()->get_pop();
  int pop_size = strain->get_population()->get_pop_size();
  
  // See if there are changes to primary_cases_per_day from primary_cases_map
  int primary_cases_per_day = primary_cases_map->get_value_for_timestep(day);
  
  // Attempt to infect primary_cases_per_day.
  // This represents external sources of infection.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  for (int i = 0; i < primary_cases_per_day; i++) {
    int n = IRAND(0, pop_size-1);
    if (pop[n]->get_strain_status(strain->get_id()) == 'S') {
      // primary infections are a special case that bypasses
      // Strain::attempt_infection(), meaning we don't roll the dice to see if 
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
    int n;
    Place *schedule[100];
    p->update_schedule(day);
    p->get_schedule(&n, schedule);
    for (int j = 0; j < n; j++) {
      Place * place = schedule[j];
      if (place && place->is_open(day) && place->should_be_open(day, strain->get_id())) {
        places.insert(place);
      }
    }
  }
  
  if (Verbose) {
    fprintf(Statusfp, "Number of infectious places = %d\n", (int) places.size());
  }
  
  // infect visitors to infectious locations:
  if (Random_location_order) {
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
  }
  else {
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != CLASSROOM) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != SCHOOL) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != OFFICE) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != WORKPLACE) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != HOSPITAL) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != NEIGHBORHOOD) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
    for (it = places.begin(); it != places.end(); it++ ) {
      Place * place = *it;
      if (place->get_type() != HOUSEHOLD) continue;
      if (Verbose > 1) {
	fprintf(Statusfp, "\nspread strain %i in location: %d\n", strain->get_id(), place->get_id()); fflush(Statusfp);
      }
      place->spread_infection(day, strain->get_id());
    }
  }

}

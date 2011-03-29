/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Epidemic.cc
//

#include "Epidemic.h"
#include "Disease.h"

#include <stdio.h>
#include <new>
#include <iostream>
using namespace std;
#include "Random.h"
#include "Params.h"
#include "Person.h"
#include "Global.h"
#include "Infection.h"
#include "Place_List.h"
#include "Place.h"
#include "Timestep_Map.h"

extern int V_count;

Epidemic::Epidemic(Disease *str, Timestep_Map* _primary_cases_map) {
  disease = str;
  id = disease->get_id();
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  new_cases = new int [Days];

  int places = Places.get_number_of_places();
  inf_households.reserve(places);
  inf_neighborhoods.reserve(places);
  inf_classrooms.reserve(places);
  inf_schools.reserve(places);
  inf_workplaces.reserve(places);
  inf_offices.reserve(places);
}

Epidemic::~Epidemic() {
  delete primary_cases_map;
}

void Epidemic::reset() {
  if (Verbose) {
    fprintf(Statusfp, "epidemic %d reset started\n", id);
    fflush(Statusfp);
  }
  infected.clear();
  infectious.clear();
  inf_households.clear();
  inf_neighborhoods.clear();
  inf_classrooms.clear();
  inf_schools.clear();
  inf_workplaces.clear();
  inf_offices.clear();
  attack_rate = 0.0;
  total_incidents = 0;
  total_clinical_incidents = 0;
  r_index = V_count = S_count = C_count = c_count = 0;
  E_count = I_count = i_count = R_count = r_count = M_count = 0;
  if (Verbose) {
    fprintf(Statusfp, "epidemic %d reset finished\n", id);
    fflush(Statusfp);
  }
}

void Epidemic::update_stats(int day) {
  if (day == 0) {
    N_init = N = disease->get_population()->get_pop_size();
  }

  clinical_incidents = c_count;
  incident_infections = C_count;
  vaccine_acceptance = V_count;
  V_count = 0;
  new_cases[day] = incident_infections;
  total_incidents += incident_infections;
  total_clinical_incidents += clinical_incidents;
  attack_rate = (100.0*total_incidents)/N_init;
  clinical_attack_rate = (100.0*total_clinical_incidents)/N_init;

  // get reproductive rate for those infected max_days ago;
  int rday = day - disease->get_max_days();
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

void Epidemic::print_stats(int day) {
  fprintf(Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	  day, id, S_count, E_count, I_count+i_count,
	  I_count, R_count+r_count, M_count);
  fprintf(Outfp,
	  "C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	  C_count, N, attack_rate, clinical_incidents,
	  vaccine_acceptance, RR,NR, clinical_attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	    day, id, S_count, E_count, I_count+i_count,
	    I_count, R_count+r_count, M_count);
    fprintf(Statusfp,
	    "C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f\n",
	    C_count, N, attack_rate, clinical_incidents,
	    vaccine_acceptance, RR,NR, clinical_attack_rate);
    fflush(Statusfp);
  }
  C_count = c_count = 0;
}



void Epidemic::add_infectious_place(Place *place, char type) {
  switch (type) {
  case HOUSEHOLD:
    inf_households.push_back(place);
    break;
    
  case NEIGHBORHOOD:
    inf_neighborhoods.push_back(place);
    break;
    
  case CLASSROOM:
    inf_classrooms.push_back(place);
    break;
    
  case SCHOOL:
    inf_schools.push_back(place);
    break;
    
  case WORKPLACE:
    inf_workplaces.push_back(place);
    break;
    
  case OFFICE:
    inf_offices.push_back(place);
    break;
  }
}

void Epidemic::get_infectious_places(int day) {
  vector<Person *>::iterator itr;
  vector<Place *>::iterator it;

  int places = Places.get_number_of_places();
  for (int p = 0; p < places; p++) {
    Place * place = Places.get_place_at_position(p);
    if (place->is_open(day) && place->should_be_open(day, id)) {
      switch (place->get_type()) {
      case HOUSEHOLD:
	inf_households.push_back(place);
	break;
	
      case NEIGHBORHOOD:
	inf_neighborhoods.push_back(place);
	break;
	
      case CLASSROOM:
	inf_classrooms.push_back(place);
	break;
	
      case SCHOOL:
	inf_schools.push_back(place);
	break;
	
      case WORKPLACE:
	inf_workplaces.push_back(place);
	break;
	
      case OFFICE:
	inf_offices.push_back(place);
	break;
      }
    }
  }
}
  

void Epidemic::update(Date *sim_start_date, int day){
  vector<Person *>::iterator itr;
  vector<Place *>::iterator it;
  Population *pop = disease->get_population();
  N = pop->get_pop_size();
  
  // See if there are changes to primary_cases_per_day from primary_cases_map
  int primary_cases_per_day = primary_cases_map->get_value_for_timestep(day, Epidemic_offset);
  
  // Attempt to infect primary_cases_per_day.
  // This represents external sources of infection.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  for (int i = 0; i < primary_cases_per_day; i++) {
    int n = IRAND(0, N-1);
    Person * person = pop->get_person(n);
    if (person->get_disease_status(id) == 'S') {
      Infection * infection = new Infection(disease, NULL, person, NULL, day);
      person->become_exposed(infection);
    }
  }
  
  // get_infectious_places(day);

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
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_schools.begin(); it != inf_schools.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_offices.begin(); it != inf_offices.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_workplaces.begin(); it != inf_workplaces.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_neighborhoods.begin(); it != inf_neighborhoods.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_households.begin(); it != inf_households.end(); it++ ) {
    Place * place = *it;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id()); fflush(Statusfp);
    }
    place->spread_infection(day, id);
  }
  inf_households.clear();
  inf_neighborhoods.clear();
  inf_classrooms.clear();
  inf_schools.clear();
  inf_workplaces.clear();
  inf_offices.clear();
}

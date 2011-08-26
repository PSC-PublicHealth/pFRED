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
#include "Multistrain_Timestep_Map.h"
#include "Transmission.h"
#include "Date.h"
#include "Grid.h"
#include "Household.h"
#include "Utils.h"

extern int V_count;

Epidemic::Epidemic(Disease *str, Timestep_Map* _primary_cases_map) {
  disease = str;
  id = disease->get_id();
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  new_cases = new int [Global::Days];

  int places = Global::Places.get_number_of_places();
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

void Epidemic::clear() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "clear epidemic %d started\n", id);
    fflush(Global::Statusfp);
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
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "clear epidemic %d finished\n", id);
    fflush(Global::Statusfp);
  }
}

void Epidemic::update_stats(int day) {
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp, "epidemic update\n");
    fflush(Global::Statusfp);
  }

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
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp, "epidemic update finished\n");
    fflush(Global::Statusfp);
  }

}

void Epidemic::print_stats(int day) {
  fprintf(Global::Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	  day, id, S_count, E_count, I_count+i_count,
	  I_count, R_count+r_count, M_count);
  fprintf(Global::Outfp,
	  "C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f  ",
	  C_count, N, attack_rate, clinical_incidents,
	  vaccine_acceptance, RR,NR, clinical_attack_rate);
  fprintf(Global::Outfp, "%s %s Year %d Week %d\n",
      Global::Sim_Date->get_day_of_week_string(day).c_str(),
      Global::Sim_Date->get_YYYYMMDD(day).c_str(),
      Global::Sim_Date->get_epi_week_year(day),
	    Global::Sim_Date->get_epi_week(day));
  fflush(Global::Outfp);
  
  if (Global::Verbose) {
    fprintf(Global::Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	    day, id, S_count, E_count, I_count+i_count,
	    I_count, R_count+r_count, M_count);
    fprintf(Global::Statusfp,
	    "C %7d  N %7d  AR %5.2f  CI %7d V %7d RR %4.2f NR %d  CAR %5.2f  ",
	    C_count, N, attack_rate, clinical_incidents,
	    vaccine_acceptance, RR,NR, clinical_attack_rate);
    fprintf(Global::Statusfp, "%s %s Year %d Week %d\n",
      Global::Sim_Date->get_day_of_week_string(day).c_str(),
      Global:: Sim_Date->get_YYYYMMDD(day).c_str(),
      Global::Sim_Date->get_epi_week_year(day),
      Global::Sim_Date->get_epi_week(day));
    fflush(Global::Statusfp);
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

  int places = Global::Places.get_number_of_places();
  for (int p = 0; p < places; p++) {
    Place * place = Global::Places.get_place_at_position(p);
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
  
//void Epidemic::update(Date *sim_start_date, int day){
void Epidemic::update(int day){
  vector<Person *>::iterator itr;
  vector<Place *>::iterator it;
  Population *pop = disease->get_population();
  N = pop->get_pop_size();

  Multistrain_Timestep_Map * ms_map = ( ( Multistrain_Timestep_Map * ) primary_cases_map );

  for (Multistrain_Timestep_Map::iterator ms_map_itr = ms_map->begin(); ms_map_itr != ms_map->end(); ms_map_itr++) {

    Multistrain_Timestep_Map::Multistrain_Timestep * mst = *ms_map_itr;
    
    if ( mst->is_applicable( day, Global::Epidemic_offset ) ) {
      
      int extra_attempts = 1000;
      int successes = 0;

      vector < Person * > people;
       
      if ( mst->has_location() ) {
        vector < Place * > households = Global::Cells->get_households_by_distance(mst->get_lat(), mst->get_lon(), mst->get_radius());
        for (vector < Place * >::iterator hi = households.begin(); hi != households.end(); hi++) {
          vector <Person *> hs = ((Household *) (*hi))->get_inhabitants();  
          // This cast is ugly and should be fixed.  
          // Problem is that Place::update (which clears susceptible list for the place) is called before Epidemic::update.
          // If households were stored as Household in the Cell (rather than the base Place class) this wouldn't be needed.
          people.insert(people.end(), hs.begin(), hs.end());
        }
      }     

      for ( int i = 0; i < mst->get_num_seeding_attempts(); i++ ) {  
        int r, n;
        Person * person;

        // each seeding attempt is independent
        if ( mst->get_seeding_prob() < 1 ) { 
          if ( RANDOM() > mst->get_seeding_prob() ) { continue; }
        }
        // if a location is specified in the timestep map select from that area, otherwise pick a person from the entire population
        if ( mst->has_location() ) {
          r = IRAND( 0, people.size()-1 );
          person = people[r];
        } else {
          n = IRAND(0, N-1);
          person = pop->get_person(n);
        }

        if (person == NULL) { // nobody home
          Utils::fred_warning("Person selected for seeding in Epidemic update is NULL.");
          continue;
        }

        if (person->get_disease_status(id) == 'S') {
          Transmission *transmission = new Transmission(NULL, NULL, day);
          transmission->setInitialLoads(disease->getPrimaryLoads(day));
          person->getInfected(this->disease, transmission);
          successes++;
        }

        if (successes < mst->get_min_successes() && i == (mst->get_num_seeding_attempts() - 1) && extra_attempts > 0 ) {
          extra_attempts--;
          i--;
        }
      }

      if (successes < mst->get_min_successes()) {
        Utils::fred_warning(
            "A minimum of %d successes was specified, but only %d successful transmissions occurred.",
            mst->get_min_successes(),successes);
      }
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
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "Number of infectious places = %d\n", infectious_places);
    fflush(Global::Statusfp);
  }
  
  for (it = inf_schools.begin(); it != inf_schools.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_classrooms.begin(); it != inf_classrooms.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_workplaces.begin(); it != inf_workplaces.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_offices.begin(); it != inf_offices.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_neighborhoods.begin(); it != inf_neighborhoods.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
    }
    place->spread_infection(day, id);
  }
  for (it = inf_households.begin(); it != inf_households.end(); it++ ) {
    Place * place = *it;
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "\nspread disease %i in place: %d\n", id, place->get_id());
      fflush(Global::Statusfp);
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

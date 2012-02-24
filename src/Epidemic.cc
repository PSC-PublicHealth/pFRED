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
#include "Seasonality.h"

Epidemic::Epidemic(Disease *dis, Timestep_Map* _primary_cases_map) {
  disease = dis;
  id = disease->get_id();
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  new_cases = new int [Global::Days];
  infectees = new int [Global::Days];
  for (int i = 0; i < Global::Days; i++) {
    new_cases[i] = infectees[i] = 0;
  }

  int places = Global::Places.get_number_of_places();
  inf_households.reserve(places);
  inf_neighborhoods.reserve(places);
  inf_classrooms.reserve(places);
  inf_schools.reserve(places);
  inf_workplaces.reserve(places);
  inf_offices.reserve(places);
  susceptible_list.clear();
  infectious_list.clear();
  inf_households.clear();
  inf_neighborhoods.clear();
  inf_classrooms.clear();
  inf_schools.clear();
  inf_workplaces.clear();
  inf_offices.clear();
  attack_rate = 0.0;
  total_incidents = 0;
  clinical_incidents = 0;
  total_clinical_incidents = 0;
  incident_infections = 0;
  symptomatic_count = 0;
  exposed_count = removed_count = immune_count = 0;
}

Epidemic::~Epidemic() {
  delete primary_cases_map;
}

void Epidemic::become_susceptible(Person *person) {
  susceptible_list.insert(make_pair(person,person->get_id()));
}

void Epidemic::become_unsusceptible(Person *person) {
  int n = susceptible_list.erase(make_pair(person,person->get_id()));
  if (n == 0) {
    Utils::fred_verbose(0,"WARNING: become_unsusc: person %d not removed from susceptible_list\n",person->get_id());
  }
}

void Epidemic::become_exposed(Person *person) {
  exposed_count++;
  incident_infections++;
}

void Epidemic::become_infectious(Person *person) {
  exposed_count--;
  infectious_list.insert(make_pair(person,person->get_id()));
}

void Epidemic::become_uninfectious(Person *person) {
  int n = infectious_list.erase(make_pair(person,person->get_id()));
  if (n == 0) {
    Utils::fred_verbose(0,"WARNING: become_uninf: person %d not removed from infectious_list\n",person->get_id());
  }
}

void Epidemic::become_symptomatic(Person *person) {
  symptomatic_count++;
  clinical_incidents++;
}

void Epidemic::become_removed(Person *person, bool susceptible, bool infectious, bool symptomatic) {
  if (susceptible) {
    int n = susceptible_list.erase(make_pair(person,person->get_id()));
    if (n == 0) {
      Utils::fred_verbose(0,"WARNING: become_removed: person %d not removed from susceptible_list\n",person->get_id());
    }
    else {
      if (Global::Verbose > 1) printf("OK: become_removed: person %d removed from susceptible_list\n",person->get_id());
    }
  }
  if (infectious) {
    int n = infectious_list.erase(make_pair(person,person->get_id()));
    if (n == 0) {
      Utils::fred_verbose(0,"WARNING: become_removed: person %d not removed from infectious_list\n",person->get_id());
    }
    else {
      if (Global::Verbose > 1) printf("OK: become_removed: person %d removed from infectious_list\n",person->get_id());
    }
  }
  if (symptomatic)
    symptomatic_count--;
  removed_count++;
}

void Epidemic::become_immune(Person *person, bool susceptible, bool infectious, bool symptomatic) {
  if (susceptible)
    susceptible_list.erase(make_pair(person,person->get_id()));
  if (infectious)
    infectious_list.erase(make_pair(person,person->get_id()));
  if (symptomatic)
    symptomatic_count--;
  immune_count++;
}

void Epidemic::print_stats(int day) {
  Utils::fred_verbose(1, "epidemic update stats\n");

  if (day == 0) {
    N_init = N = disease->get_population()->get_pop_size();
  }

  new_cases[day] = incident_infections;
  total_incidents += incident_infections;
  total_clinical_incidents += clinical_incidents;
  attack_rate = (100.0*total_incidents)/N_init;
  clinical_attack_rate = (100.0*total_clinical_incidents)/N_init;

  // get reproductive rate for the cohort exposed RR_delay days ago
  // unless RR_delay == 0
  RR = 0.0;	    // reproductive rate for a fixed cohort of infectors
  cohort_size = 0; // size of the cohort exposed on cohort_day
  if (0 < Global::RR_delay && Global::RR_delay <= day) {
    int cohort_day = day - Global::RR_delay;  // exposure day for cohort
    cohort_size = new_cases[cohort_day];	// size of cohort
    if (cohort_size > 0) {  // compute reproductive rate for this cohort
      RR = (double)infectees[cohort_day] /(double)cohort_size;
    }
  }

  int susceptible_count = susceptible_list.size();
  int infectious_count = infectious_list.size();
  double average_seasonality_multiplier = 1.0;
  if (Global::Enable_Seasonality) {
    average_seasonality_multiplier = Global::Clim->get_average_seasonality_multiplier(disease->get_id());
  }

  fprintf(Global::Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	  day, id, susceptible_count, exposed_count, infectious_count,
	  symptomatic_count, removed_count, immune_count);
  fprintf(Global::Outfp,
	  "C %7d  N %7d  AR %5.2f  CI %7d RR %4.2f NR %d  CAR %5.2f  ",
	  incident_infections, N, attack_rate, clinical_incidents,
	  RR, cohort_size, clinical_attack_rate);
  fprintf(Global::Outfp, "%s %s Year %d Week %d",
      Global::Sim_Date->get_day_of_week_string(day).c_str(),
      Global::Sim_Date->get_YYYYMMDD(day).c_str(),
      Global::Sim_Date->get_epi_week_year(day),
	    Global::Sim_Date->get_epi_week(day));
  if (Global::Enable_Seasonality) {
    fprintf(Global::Outfp, " SM %2.4f\n", average_seasonality_multiplier);
  } else {
    fprintf(Global::Outfp, "\n");
  }
  fflush(Global::Outfp);
  
  if (Global::Verbose) {
    fprintf(Global::Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
	    day, id, susceptible_count, exposed_count, infectious_count,
	    symptomatic_count, removed_count, immune_count);
    fprintf(Global::Statusfp,
	    "C %7d  N %7d  AR %5.2f  CI %7d RR %4.2f NR %d  CAR %5.2f  ",
	    incident_infections, N, attack_rate, clinical_incidents,
	    RR,cohort_size, clinical_attack_rate);
    fprintf(Global::Statusfp, "%s %s Year %d Week %d",
      Global::Sim_Date->get_day_of_week_string(day).c_str(),
      Global::Sim_Date->get_YYYYMMDD(day).c_str(),
      Global::Sim_Date->get_epi_week_year(day),
      Global::Sim_Date->get_epi_week(day));
    if (Global::Enable_Seasonality) {
      fprintf(Global::Statusfp, " SM %2.4f\n", average_seasonality_multiplier);
    } else {
      fprintf(Global::Statusfp, "\n");
    }
    fflush(Global::Statusfp);
  }
  // prepare for next day
  incident_infections = clinical_incidents = 0;
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
  
void Epidemic::get_primary_infections(int day){
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
	} else if (Global::Seed_by_age) {
	  person = pop->select_random_person_by_age(Global::Seed_age_lower_bound, Global::Seed_age_upper_bound);
        } else {
          n = IRAND(0, N-1);
          person = pop->get_person(n);
        }

        if (person == NULL) { // nobody home
          Utils::fred_warning("Person selected for seeding in Epidemic update is NULL.\n");
          continue;
        }

        if (person->get_health()->is_susceptible(id)) {
          Transmission *transmission = new Transmission(NULL, NULL, day);
          transmission->setInitialLoads(disease->getPrimaryLoads(day));
          person->become_exposed(this->disease, transmission);
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

}

void Epidemic::transmit(int day){
  vector<Person *>::iterator itr;
  vector<Place *>::iterator it;
  Population *pop = disease->get_population();
  N = pop->get_pop_size();

  // import infections from unknown sources
  get_primary_infections(day);

  int infectious_places;
  infectious_places = (int) inf_households.size();
  infectious_places += (int) inf_neighborhoods.size();
  infectious_places += (int) inf_schools.size();
  infectious_places += (int) inf_classrooms.size();
  infectious_places += (int) inf_workplaces.size();
  infectious_places += (int) inf_offices.size();
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "Number of infectious places = %d\n",
	    infectious_places);
    fprintf(Global::Statusfp, "Number of infectious households = %d\n",
	    (int) inf_households.size());
    fprintf(Global::Statusfp, "Number of infectious neighborhoods = %d\n",
	    (int) inf_neighborhoods.size());
    fprintf(Global::Statusfp, "Number of infectious schools = %d\n",
	    (int) inf_schools.size());
    fprintf(Global::Statusfp, "Number of infectious classrooms = %d\n",
	    (int) inf_classrooms.size());
    fprintf(Global::Statusfp, "Number of infectious workplaces = %d\n",
	    (int) inf_workplaces.size());
    fprintf(Global::Statusfp, "Number of infectious offices = %d\n",
	    (int) inf_offices.size());
    fflush(Global::Statusfp);
  }
  
  for (it = inf_schools.begin(); it != inf_schools.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  for (it = inf_classrooms.begin(); it != inf_classrooms.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  for (it = inf_workplaces.begin(); it != inf_workplaces.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  for (it = inf_offices.begin(); it != inf_offices.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  for (it = inf_neighborhoods.begin(); it != inf_neighborhoods.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  for (it = inf_households.begin(); it != inf_households.end(); it++ ) {
    Place * place = *it;
    place->spread_infection(day, id);
  }
  inf_households.clear();
  inf_neighborhoods.clear();
  inf_classrooms.clear();
  inf_schools.clear();
  inf_workplaces.clear();
  inf_offices.clear();
}

void Epidemic::update(int day) {
  Activities::update(day);
  for (int d = 0; d < Global::Diseases; d++) {
    Disease * disease = Global::Pop.get_disease(d);
    Epidemic * epidemic = disease->get_epidemic();
    epidemic->find_infectious_places(day, d);
    epidemic->add_susceptibles_to_infectious_places(day, d);
    epidemic->transmit(day);
  }
}

void Epidemic::find_infectious_places(int day, int dis) {
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "find_infectious_places entered\n");
    fflush(Global::Statusfp);
  }

  set <person_pair>::iterator it;
  for (it = infectious_list.begin(); it != infectious_list.end(); it++) {
    Person * person = it->first;
    person->get_activities()->update_infectious_activities(day, dis);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "find_infectious_places finished\n");
    fflush(Global::Statusfp);
  }
}

void Epidemic::add_susceptibles_to_infectious_places(int day, int dis) {
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "add_susceptibles_to_infectious_places entered\n");
    fflush(Global::Statusfp);
  }

  set <person_pair>::iterator it;
  for (it = susceptible_list.begin(); it != susceptible_list.end(); it++) {
    Person * person = it->first;;
    person->get_activities()->update_susceptible_activities(day, dis);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "add_susceptibles_to_infectious_places finished\n");
    fflush(Global::Statusfp);
  }
}


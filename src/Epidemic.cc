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
#include <vector>
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
#include "Workplace.h"

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
  daily_infections_list.reserve(N_init);
  daily_infections_list.clear();
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
  daily_infections_list.push_back(person);
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
  else {
    N = disease->get_population()->get_pop_size();
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

  // basic epidemic measures
  int susceptible_count = susceptible_list.size();
  int infectious_count = infectious_list.size();

  /*
  // date related information
  Utils::fred_report("Day %3d %s %s Yr %d Wk %2d",
	  day, Global::Sim_Date->get_day_of_week_string(day).c_str(),
	  Global::Sim_Date->get_YYYYMMDD(day).c_str(),
	  Global::Sim_Date->get_epi_week_year(day),
	  Global::Sim_Date->get_epi_week(day));

  Utils::fred_report(
	  " Str %d N %7d S %7d E %7d I %7d I_s %7d R %7d M %d",
	  id, N, susceptible_count, exposed_count, infectious_count,
	  symptomatic_count, removed_count, immune_count);
  Utils::fred_report(
	  " C %7d CI %7d AR %5.2f CAR %5.2f RR %4.2f",
	  incident_infections, clinical_incidents, attack_rate,
	  clinical_attack_rate, RR);

  */

  Utils::fred_report("Day %3d  Str %d  S %7d  E %7d  I %7d  I_s %7d  R %7d  M %7d  ",
		     day, id, susceptible_count, exposed_count, infectious_count,
		     symptomatic_count, removed_count, immune_count);

  
  Utils::fred_report("C %7d  N %7d  AR %5.2f  CI %7d RR %4.2f NR %d  CAR %5.2f  ",
		     incident_infections, N, attack_rate, clinical_incidents,
		     RR, cohort_size, clinical_attack_rate);
  
  Utils::fred_report("%s %s Year %d Week %d",
		   Global::Sim_Date->get_day_of_week_string(day).c_str(),
		   Global::Sim_Date->get_YYYYMMDD(day).c_str(),
		   Global::Sim_Date->get_epi_week_year(day),
		   Global::Sim_Date->get_epi_week(day));

  // optional reports
  if (Global::Enable_Seasonality) {
    Utils::fred_report(" SM %2.4f",
	    Global::Clim->get_average_seasonality_multiplier(id));
  }
  if (Global::Report_Presenteeism) { report_presenteeism(day); }
  if (Global::Report_Place_Of_Infection) { report_place_of_infection(day); }
  if (Global::Report_Age_Of_Infection) { report_age_of_infection(day); }

  // terminate daily output line
  Utils::fred_report("\n");


  // prepare for next day
  incident_infections = clinical_incidents = 0;
  daily_infections_list.clear();
}

void::Epidemic::report_age_of_infection(int day) {
  int age_count[21];				// age group counts
  for (int i = 0; i < 21; i++) age_count[i] = 0;
  for (int i = 0; i < incident_infections; i++) {
    Person * infectee = daily_infections_list[i];
    int age_group = infectee->get_age() / 5;
    if (age_group > 20) age_group = 20;
    age_count[age_group]++;
  }
  Utils::fred_log("\nDay %d INF_AGE: ", day);
  for (int i = 0; i <= 20; i++) {
    Utils::fred_report(" A%d %d", i*5, age_count[i]);
  }
  Utils::fred_log("\n");
}

void::Epidemic::report_place_of_infection(int day) {
  // type of place of infection
  int X = 0;
  int H = 0;
  int N = 0;
  int S = 0;
  int C = 0;
  int W = 0;
  int O = 0;
  for (int i = 0; i < incident_infections; i++) {
    Person * infectee = daily_infections_list[i];
    char c = infectee->get_infected_place_type(id);
    switch(c) {
    case 'X': X++; break;
    case 'H': H++; break;
    case 'N': N++; break;
    case 'S': S++; break;
    case 'C': C++; break;
    case 'W': W++; break;
    case 'O': O++; break;
    }
  }
  Utils::fred_log("\nDay %d INF_PLACE: ", day);
  Utils::fred_report(" X %d H %d Nbr %d Sch %d", X, H, N, S);
  Utils::fred_report(" Cls %d Wrk %d Off %d ", C, W, O);
  Utils::fred_log("\n");
}

void Epidemic::report_presenteeism(int day) {
  // daily totals
  int infections_in_pop = 0;
  int presenteeism_small = 0;
  int presenteeism_med = 0;
  int presenteeism_large = 0;
  int presenteeism_xlarge = 0;
  int presenteeism_small_with_sl = 0;
  int presenteeism_med_with_sl = 0;
  int presenteeism_large_with_sl = 0;
  int presenteeism_xlarge_with_sl = 0;
  int infections_at_work = 0;

  // company size limits
  static int small;
  static int medium;
  static int large;
  if (day == 0) {
    small = Workplace::get_small_workplace_size();
    medium = Workplace::get_medium_workplace_size();
    large = Workplace::get_large_workplace_size();
  }

  for (int i = 0; i < incident_infections; i++) {
    Person * infectee = daily_infections_list[i];
    char c = infectee->get_infected_place_type(id);
    infections_in_pop++;

    // presenteeism requires that place of infection is work or office
    if (c != 'W' && c != 'O') {
      continue;
    }
    infections_at_work++;

    // get the work place size (note we don't care about the office size)
    Place * work = infectee->get_workplace();
    assert(work != NULL);
    int size = work->get_size();

    // presenteeism requires that the infector have symptoms
    Person * infector = infectee->get_infector(this->id);
    assert(infector != NULL);
    if (infector->is_symptomatic()) {

      // determine whether sick leave was available to infector
      bool infector_has_sick_leave = infector->is_sick_leave_available();

      if (size < small) {			// small workplace
	presenteeism_small++;
	if (infector_has_sick_leave)
	  presenteeism_small_with_sl++;
      } else if (size < medium) {		// medium workplace
	presenteeism_med++;
	if (infector_has_sick_leave)
	  presenteeism_med_with_sl++;
      } else if (size < large) {		// large workplace
	presenteeism_large++;
	if (infector_has_sick_leave)
	  presenteeism_large_with_sl++;
      } else {					// xlarge workplace
	presenteeism_xlarge++;
	if (infector_has_sick_leave)
	  presenteeism_xlarge_with_sl++;
      }
    }
  } // end loop over infectees

  // raw counts
  int presenteeism = presenteeism_small + presenteeism_med
    + presenteeism_large + presenteeism_xlarge;
  int presenteeism_with_sl = presenteeism_small_with_sl + presenteeism_med_with_sl
    + presenteeism_large_with_sl + presenteeism_xlarge_with_sl;
  
  Utils::fred_log("\nDay %d PRESENTEE: ",day);
  Utils::fred_report(" small %d ", presenteeism_small);
  Utils::fred_report("small_n %d ", Workplace::get_workers_in_small_workplaces());
  Utils::fred_report("med %d ", presenteeism_med);
  Utils::fred_report("med_n %d ", Workplace::get_workers_in_medium_workplaces());
  Utils::fred_report("large %d ", presenteeism_large);
  Utils::fred_report("large_n %d ", Workplace::get_workers_in_large_workplaces());
  Utils::fred_report("xlarge %d ", presenteeism_xlarge);
  Utils::fred_report("xlarge_n %d ", Workplace::get_workers_in_xlarge_workplaces());
  Utils::fred_report("pres %d ", presenteeism);
  Utils::fred_report("pres_sl %d ", presenteeism_with_sl);
  Utils::fred_report("inf_at_work %d ", infections_at_work);
  Utils::fred_report("tot_emp %d ", Workplace::get_total_workers());
  Utils::fred_log("N %d\n", N);
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


/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
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
#include "Evolution.h"
#include "Workplace.h"

Epidemic::Epidemic(Disease *dis, Timestep_Map* _primary_cases_map) {
  disease = dis;
  id = disease->get_id();
  primary_cases_map = _primary_cases_map;
  primary_cases_map->print(); 
  daily_cohort_size = new int [Global::Days];
  number_infected_by_cohort = new int [Global::Days];
  for (int i = 0; i < Global::Days; i++) {
    daily_cohort_size[i] = 0;
    number_infected_by_cohort[i] = 0;
  }

  inf_households.reserve( Global::Places.get_number_of_places( HOUSEHOLD ) );
  inf_neighborhoods.reserve( Global::Places.get_number_of_places( NEIGHBORHOOD ) );
  inf_classrooms.reserve( Global::Places.get_number_of_places( CLASSROOM ) );
  inf_schools.reserve( Global::Places.get_number_of_places( SCHOOL ) );
  inf_workplaces.reserve( Global::Places.get_number_of_places( WORKPLACE ) );
  inf_offices.reserve( Global::Places.get_number_of_places( OFFICE ) );
  
  inf_households.clear();
  inf_neighborhoods.clear();
  inf_classrooms.clear();
  inf_schools.clear();
  inf_workplaces.clear();
  inf_offices.clear();
  
  susceptible_people = 0;
  exposed_people = 0;
  infectious_people = 0;
  removed_people = 0;
  immune_people = 0;

  people_becoming_infected_today = 0;
  total_people_ever_infected = 0;

  people_becoming_symptomatic_today = 0;
  people_with_current_symptoms = 0;
  total_people_ever_symptomatic = 0;

  attack_ratio = 0.0;
  symptomatic_attack_ratio = 0.0;

  incidence = 0;
  prevalence = 0.0;

  place_person_list_reserve_size = 1;
  daily_infections_list.clear();
}

Epidemic::~Epidemic() {
  delete primary_cases_map;
}

void Epidemic::become_susceptible(Person *person) {
  // operations on bloque (underlying container for population) are thread-safe
  Global::Pop.set_mask_by_index( fred::Susceptible, person->get_pop_index() );
}

void Epidemic::become_unsusceptible(Person *person) {
  // operations on bloque (underlying container for population) are thread-safe
  if ( Global::Pop.check_mask_by_index( fred::Susceptible, person->get_pop_index() ) ) {
    Global::Pop.clear_mask_by_index( fred::Susceptible, person->get_pop_index() );
  }
  else {
    FRED_VERBOSE( 0, "WARNING: become_unsusceptible: person %d not removed from \
        susceptible_list\n", person->get_id() );
  }
}

void Epidemic::become_exposed(Person *person) {
  #pragma omp atomic
  ++people_becoming_infected_today;
  #pragma omp atomic
  ++exposed_people;

  // TODO the daily infections list may end up containing defunct pointers if
  // enable_deaths is in effect (whether or not we are running in parallel mode).
  // Make daily reports and purge list after each report to fix this.
  fred::Spin_Lock lock( spin_mutex );
  daily_infections_list.push_back(person);
}

void Epidemic::become_infectious(Person *person) {
  #pragma omp atomic
  exposed_people--;
  // operations on bloque (underlying container for population) are thread-safe
  Global::Pop.set_mask_by_index( fred::Infectious, person->get_pop_index() );
}

void Epidemic::become_uninfectious(Person *person) {
  // operations on bloque (underlying container for population) are thread-safe
  if ( Global::Pop.check_mask_by_index( fred::Infectious, person->get_pop_index() ) ) {
    Global::Pop.clear_mask_by_index( fred::Infectious, person->get_pop_index() );
  }
  else {
    FRED_VERBOSE( 0, "WARNING: become_uninfectious: person %d not removed from \
        infectious_list\n",person->get_id() );
  }
}

void Epidemic::become_symptomatic(Person *person) {
  #pragma omp atomic
  ++people_with_current_symptoms;
  #pragma omp atomic
  ++people_becoming_symptomatic_today;
}

void Epidemic::become_removed(Person *person, bool susceptible, bool infectious, bool symptomatic) {
  // operations on bloque (underlying container for population) are thread-safe
  if (susceptible) {
    if ( Global::Pop.check_mask_by_index( fred::Susceptible, person->get_pop_index() ) ) {
      Global::Pop.clear_mask_by_index( fred::Susceptible, person->get_pop_index() );
      FRED_VERBOSE( 1, "OK: become_removed: person %d removed from \
          susceptible_list\n",person->get_id() );
    }
    else {
      FRED_VERBOSE( 0, "WARNING: become_removed: person %d not removed \
          from susceptible_list\n",person->get_id() );
    }
  }
  if (infectious) {
    if ( Global::Pop.check_mask_by_index( fred::Infectious, person->get_pop_index() ) ) {
      Global::Pop.clear_mask_by_index( fred::Infectious, person->get_pop_index() );
      FRED_VERBOSE( 1, "OK: become_removed: person %d removed from \
          infectious_list\n",person->get_id() );
    }
    else {
      FRED_VERBOSE( 0, "WARNING: become_removed: person %d not removed from \
          infectious_list\n",person->get_id() );
    }
  }
  if (symptomatic) {
    #pragma omp atomic
    --( people_with_current_symptoms );
  }
  #pragma omp atomic
  ++( removed_people );
}

void Epidemic::become_immune(Person *person, bool susceptible, bool infectious, bool symptomatic) {
  if (susceptible) {
    if ( Global::Pop.check_mask_by_index( fred::Susceptible, person->get_pop_index() ) ) {
      Global::Pop.clear_mask_by_index( fred::Susceptible, person->get_pop_index() );
      FRED_VERBOSE( 1, "OK: become_immune: person %d removed from \
          susceptible_list\n",person->get_id() );
    }
    else {
      FRED_VERBOSE( 0, "WARNING: become_immune: person %d not removed from \
          susceptible_list\n",person->get_id() );
    }
  }
  if (infectious) {
    if ( Global::Pop.check_mask_by_index( fred::Infectious, person->get_pop_index() ) ) {
      Global::Pop.clear_mask_by_index( fred::Infectious, person->get_pop_index() );
      FRED_VERBOSE( 1, "OK: become_immune: person %d removed from \
          infectious_list\n", person->get_id() );
    }
    else {
      FRED_VERBOSE( 0, "WARNING: become_immune: person %d not removed from \
          infectious_list\n", person->get_id() );
    }
  }
  if (symptomatic) {
    #pragma omp atomic
    --( people_with_current_symptoms );
  }
  #pragma omp atomic
  ++immune_people;

  #pragma omp atomic
  ++removed_people;
}

void Epidemic::print_stats(int day) {
  FRED_VERBOSE(1, "epidemic update stats\n","");

  // set population size, and remember original pop size
  if (day == 0) { N_init = N = disease->get_population()->get_pop_size(); }
  else { N = disease->get_population()->get_pop_size(); }

  // get reproductive rate for the cohort exposed RR_delay days ago
  // unless RR_delay == 0
  daily_cohort_size[day] = people_becoming_infected_today;
  RR = 0.0;         // reproductive rate for a fixed cohort of infectors
  if (0 < Global::RR_delay && Global::RR_delay <= day) {
    int cohort_day = day - Global::RR_delay;    // exposure day for cohort
    int cohort_size = daily_cohort_size[cohort_day];        // size of cohort
    if (cohort_size > 0) {
      // compute reproductive rate for this cohort
      RR = (double)number_infected_by_cohort[cohort_day] /(double)cohort_size;
    }
  }

  susceptible_people = Global::Pop.size( fred::Susceptible ); 
  infectious_people = Global::Pop.size( fred::Infectious );

  total_people_ever_infected += people_becoming_infected_today;
  total_people_ever_symptomatic += people_becoming_symptomatic_today;

  attack_ratio = (100.0*total_people_ever_infected)/N_init;
  symptomatic_attack_ratio = (100.0*total_people_ever_symptomatic)/N_init;

  // preserve these quantities for use during the next day
  incidence = people_becoming_infected_today;
  symptomatic_incidence = people_becoming_symptomatic_today;
  prevalence_count = exposed_people + infectious_people;
  prevalence = (double) prevalence_count / (double) N;

  char buffer[ FRED_STRING_SIZE ];
  int nchar_used;
  if (Global::Report_Prevalence) {
    nchar_used = sprintf(buffer,
			 "Day %3d Date %s Wkday %s Year %d Week %2d Str %d S %7d E %7d I %7d I_s %7d R %7d M %7d P %7d C %7d CI %7d AR %5.2f CAR %5.2f RR %4.2f N %7d",
			 day, Global::Sim_Current_Date->get_YYYYMMDD().c_str(),
			 Global::Sim_Current_Date->get_day_of_week_string().c_str(), 
			 Global::Sim_Current_Date->get_epi_week_year(), 
			 Global::Sim_Current_Date->get_epi_week(),
			 id, susceptible_people, exposed_people, infectious_people,
			 people_with_current_symptoms, removed_people, immune_people,
			 prevalence_count, incidence, symptomatic_incidence,
			 attack_ratio, symptomatic_attack_ratio, RR, N);
  }
  else {
    nchar_used = sprintf(buffer,
			 "Day %3d Date %s Wkday %s Year %d Week %2d Str %d S %7d E %7d I %7d I_s %7d R %7d M %7d C %7d CI %7d AR %5.2f CAR %5.2f RR %4.2f N %7d",
			 day, Global::Sim_Current_Date->get_YYYYMMDD().c_str(),
			 Global::Sim_Current_Date->get_day_of_week_string().c_str(), 
			 Global::Sim_Current_Date->get_epi_week_year(), 
			 Global::Sim_Current_Date->get_epi_week(),
			 id, susceptible_people, exposed_people, infectious_people,
			 people_with_current_symptoms, removed_people, immune_people,
			 incidence, symptomatic_incidence,
			 attack_ratio, symptomatic_attack_ratio, RR, N);
  }

  fprintf( Global::Outfp, "%s", buffer );
  FRED_STATUS(0, "%s", buffer);

  if (Global::Enable_Seasonality) {
    double average_seasonality_multiplier = 1.0;
    average_seasonality_multiplier = Global::Clim->get_average_seasonality_multiplier(disease->get_id());
    fprintf(Global::Outfp, " SM %2.4f", average_seasonality_multiplier);
    FRED_STATUS(0, " SM %2.4f", average_seasonality_multiplier);
  } 

  // Print Residual Immunuties
  if (Global::Track_Residual_Immunity) {
    Evolution *evol = disease->get_evolution();
    for(int i=0; i<disease->get_num_strains(); i++){
      double res_immunity = 0.0;
      Population *pop = disease->get_population();
      for(int j=0; j<pop->get_pop_size(); j++){
        res_immunity = evol->residual_immunity(pop->get_person_by_index(j), i, day);
      }
      res_immunity /= pop->get_pop_size();
      if(res_immunity != 0) {
        //fprintf(Global::Outfp, " ResM_%d %lf", i, res_immunity);
        fprintf(Global::Outfp, " S_%d %lf", i, 1.0-res_immunity);
        //FRED_STATUS(0, " ResM_%d %lf", i, res_immunity);
        FRED_STATUS(0, " S_%d %lf", i, 1.0-res_immunity);
      }
    }
  }

  if (Global::Report_Presenteeism) { report_presenteeism(day); }
  if (Global::Report_Place_Of_Infection) { report_place_of_infection(day); }
  if (Global::Report_Age_Of_Infection) { report_age_of_infection(day); }
  
  fprintf(Global::Outfp, "\n");  
  fflush(Global::Outfp);

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "\n");
    fflush(Global::Statusfp);
  }

  // prepare for next day
  people_becoming_infected_today = people_becoming_symptomatic_today = 0;
  daily_infections_list.clear();
}

void::Epidemic::report_age_of_infection(int day) {
  int age_count[21];				// age group counts
  double mean_age = 0.0;
  int count_infections = 0;
  for (int i = 0; i < 21; i++) age_count[i] = 0;
  for (int i = 0; i < people_becoming_infected_today; i++) {
    Person * infectee = daily_infections_list[i];
    int age = infectee->get_age();
    mean_age += age;
    count_infections++;
    int age_group = age / 5;
    if (age_group > 20) age_group = 20;
    age_count[age_group]++;
  }
  if (count_infections > 0) { mean_age /= count_infections; }
  Utils::fred_log("\nDay %d INF_AGE: ", day);
  Utils::fred_report(" Age_at_infection %f", mean_age);
  if (Global::Report_Age_Of_Infection > 1) { 
    report_transmission_by_age_group(day); 
  }
  if (Global::Report_Age_Of_Infection > 2) {
    for (int i = 0; i <= 20; i++) {
      Utils::fred_report(" A%d %d", i*5, age_count[i]);
    }
  }
  Utils::fred_log("\n");
}

void::Epidemic::report_transmission_by_age_group(int day) {
  FILE *fp;
  char file[1024];
  sprintf(file, "%s/AGE.%d", Global::Output_directory, day);
  fp = fopen(file, "w");
  if (fp == NULL) {
    Utils::fred_abort("Can't open file to report age transmission matrix\n");
  }
  int age_count[100][100];				// age group counts
  for (int i = 0; i < 100; i++)
    for (int j = 0; j < 100; j++)
      age_count[i][j] = 0;
  int group = 1;
  int groups = 100 / group;
  for (int i = 0; i < people_becoming_infected_today; i++) {
    Person * infectee = daily_infections_list[i];
    Person * infector = daily_infections_list[i]->get_infector(0);
    if (infector == NULL) continue;
    int a1 = infector->get_age();
    int a2 = infectee->get_age();
    if (a1 > 99) a1 = 99;
    if (a2 > 99) a2 = 99;
    a1 = a1 / group;
    a2 = a2 / group;
    age_count[a1][a2]++;
  }
  for (int i = 0; i < groups; i++) {
    for (int j = 0; j < groups; j++) {
      fprintf(fp, " %d", age_count[j][i]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
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
  for (int i = 0; i < people_becoming_infected_today; i++) {
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

  for (int i = 0; i < people_becoming_infected_today; i++) {
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



void Epidemic::add_infectious_place( Place *place, char type ) {
  switch (type) {
    case HOUSEHOLD:
      {
        fred::Spin_Lock lock( household_mutex );
        inf_households.push_back(place);
      }
      break;

    case NEIGHBORHOOD:
      {
        fred::Spin_Lock lock( neighborhood_mutex );
        inf_neighborhoods.push_back(place);
      }
      break;

    case CLASSROOM:
      {
        fred::Spin_Lock lock( classroom_mutex );
        inf_classrooms.push_back(place);
      }
      break;

    case SCHOOL:
      {
        fred::Spin_Lock lock( school_mutex );
        inf_schools.push_back(place);
      }
      break;

    case WORKPLACE:
      {
        fred::Spin_Lock lock( workplace_mutex );
        inf_workplaces.push_back(place);
      }
      break;

    case OFFICE:
      {
        fred::Spin_Lock lock( office_mutex );
        inf_offices.push_back(place);
      }
      break;
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
          person = pop->get_person_by_index(n);
        }

        if (person == NULL) { // nobody home
          FRED_WARNING("Person selected for seeding in Epidemic update is NULL.\n");
          continue;
        }

        if ( person->get_health()->is_susceptible( id ) ) {
          Transmission transmission = Transmission( NULL, NULL, day );
          transmission.set_initial_loads( disease->get_primary_loads( day ) );
          person->become_exposed( disease, transmission );
          successes++;
        }

        if (successes < mst->get_min_successes() && i == (mst->get_num_seeding_attempts() - 1) && extra_attempts > 0 ) {
          extra_attempts--;
          i--;
        }
      }

      if (successes < mst->get_min_successes()) {
        FRED_WARNING("A minimum of %d successes was specified, but only %d successful transmissions occurred.",
            mst->get_min_successes(),successes);
      }
    }
  }

}

void Epidemic::transmit(int day){
  Population *pop = disease->get_population();

  // import infections from unknown sources
  get_primary_infections(day);

  int infectious_places;
  infectious_places =  (int) inf_households.size();
  infectious_places += (int) inf_neighborhoods.size();
  infectious_places += (int) inf_schools.size();
  infectious_places += (int) inf_classrooms.size();
  infectious_places += (int) inf_workplaces.size();
  infectious_places += (int) inf_offices.size();

  FRED_STATUS(0, "Number of infectious places        => %9d\n", infectious_places );
  FRED_STATUS(0, "Number of infectious households    => %9d\n", (int) inf_households.size());
  FRED_STATUS(0, "Number of infectious neighborhoods => %9d\n", (int) inf_neighborhoods.size());
  FRED_STATUS(0, "Number of infectious schools       => %9d\n", (int) inf_schools.size());
  FRED_STATUS(0, "Number of infectious classrooms    => %9d\n", (int) inf_classrooms.size());
  FRED_STATUS(0, "Number of infectious workplaces    => %9d\n", (int) inf_workplaces.size());
  FRED_STATUS(0, "Number of infectious offices       => %9d\n", (int) inf_offices.size());
  
  #pragma omp parallel
  {
    // schools (and classrooms)
    #pragma omp for schedule(dynamic,10)
    for ( int i = 0; i < inf_schools.size(); ++i ) {
      inf_schools[ i ]->spread_infection( day, id );
    }

    #pragma omp for schedule(dynamic,10)
    for ( int i = 0; i < inf_classrooms.size(); ++i ) {
      inf_classrooms[ i ]->spread_infection( day, id );
    }

    // workplaces (and offices)
    #pragma omp for schedule(dynamic,10)
    for ( int i = 0; i < inf_workplaces.size(); ++i ) {
      inf_workplaces[ i ]->spread_infection( day, id );
    }
    #pragma omp for schedule(dynamic,10)
    for ( int i = 0; i < inf_offices.size(); ++i ) {
      inf_offices[ i ]->spread_infection( day, id );
    }

    // neighborhoods (and households)
    #pragma omp for schedule(dynamic,100)
    for ( int i = 0; i < inf_neighborhoods.size(); ++i ) {
      inf_neighborhoods[ i ]->spread_infection( day, id );
    }
    #pragma omp for schedule(dynamic,100)
    for ( int i = 0; i < inf_households.size(); ++i ) {
      inf_households[ i ]->spread_infection( day, id );
    }
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
    disease->get_evolution()->update( day );
  }
}

void Epidemic::update_infectious_activities::operator() ( Person & person ) {
  person.get_activities()->update_infectious_activities( & person, day, disease_id );
}

void Epidemic::find_infectious_places( int day, int disease_id ) {
  FRED_STATUS(1, "find_infectious_places entered\n", "");

  update_infectious_activities update_functor( day, disease_id );
  Global::Pop.parallel_masked_apply( fred::Infectious, update_functor );

  FRED_STATUS(1, "find_infectious_places finished\n", "");
}

void Epidemic::update_susceptible_activities::operator() ( Person & person ) {
  person.get_activities()->update_susceptible_activities( & person, day, disease_id );
}

void Epidemic::add_susceptibles_to_infectious_places(int day, int disease_id) {
  FRED_STATUS(1, "add_susceptibles_to_infectious_places entered\n");

  update_susceptible_activities update_functor( day, disease_id );
  Global::Pop.parallel_masked_apply( fred::Susceptible, update_functor );

  FRED_STATUS(1, "add_susceptibles_to_infectious_places finished\n");
}

void Epidemic::infectious_sampler::operator() ( Person & person ) {
  if ( RANDOM() < prob ) {
    #pragma omp critical(EPIDEMIC_INFECTIOUS_SAMPLER)
    samples->push_back( &person );
  }
}

void Epidemic::get_infectious_samples(vector<Person *> &samples, double prob = 1.0) {
  if(prob > 1.0) prob = 1.0;
  if(prob <= 0.0) return;
  samples.clear();
  infectious_sampler sampler;
  sampler.samples = &samples;
  sampler.prob = prob;
  Global::Pop.parallel_masked_apply( fred::Infectious, sampler );
}


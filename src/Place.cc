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
// File: Place.cc
//

#include "Place.h"
#include "Global.h"
#include "Params.h"
#include "Person.h"
#include "Disease.h"
#include "Infection.h"
#include "Transmission.h"
#include "Date.h"
#include "Neighborhood.h"
#include "Cell.h"
#include "Seasonality.h"
#include "Utils.h"
#include "Large_Grid.h"
#include "Large_Cell.h"


void Place::setup( const char *lab, fred::geo lon, fred::geo lat, Place* cont, Population *pop ) {
  population = pop;
  id = -1;		  // actual id assigned in Place_List::add_place
  container = cont;
  strcpy(label, lab);
  longitude = lon;
  latitude = lat;
  enrollees.clear();
  N = 0;
  first_day_infectious = -1;
  last_day_infectious = -2;

  // zero out all disease-specific counts
  for ( int d = 0; d < Global::MAX_NUM_DISEASES; ++d ) {
    new_infections[ d ] = 0;
    total_infections[ d ] = 0;
    current_infections[ d ] = 0;

    new_symptomatic_infections[ d ] = 0;
    total_symptomatic_infections[ d ] = 0;
    current_symptomatic_infections[ d ] = 0;

    new_deaths[ d ] = 0;
    total_deaths[ d ] = 0;
  }

}


void Place::prepare() {
  for (int d = 0; d < Global::Diseases; d++) {
    // Following arithmetic estimates the optimal number of thread-safe states
    // to be allocated for this place, for each disease.  The number of states
    // should always be 1 <= dim <= max_num_threads.  Each state is thread-safe,
    // but increasing the number of states reduces lock-contention.  Minimizing
    // the number of states saves memory.  The values used here must be determined
    // through experimentation.  Optimal values will vary based on the particular
    // cpu, number of threads, calibrated attack rate, population density, etc...
    unsigned int dim = ( N / 1000 ) * ( fred::omp_get_max_threads() / 2 );
    dim = dim == 0 ? 1 : dim;
    dim = dim <= fred::omp_get_max_threads() ? dim : fred::omp_get_max_threads();
    // Initialize specified number of states for this disease
    place_state[ d ] = State< Place_State >( dim );
    // Clear the states for use
    place_state[ d ].clear();
  }
  update(0);
  open_date = 0;
  close_date = INT_MAX;
  if (Global::Verbose > 2) {
    printf("prepare place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}

void Place::update(int day) {
  for (int d = 0; d < Global::Diseases; d++) {
    if ( infectious_bitset.test( d ) ) {
      place_state[ d ].clear(); 
    }
    else {
      place_state[ d ].reset();
    }
    infectious_bitset.reset();
  }

  for (int d = 0; d < Global::Diseases; d++) {
    new_infections[ d ] = 0;
    current_infections[ d ] = 0;
    new_symptomatic_infections[ d ] = 0;
    current_symptomatic_infections[ d ] = 0;
    new_deaths[ d ] = 0;
  }
}

void Place::count_new_infection(Person * per, int disease_id) {
  #pragma omp atomic
  new_infections[disease_id]++; 
  #pragma omp atomic
  total_infections[disease_id]++;

  if (per->get_health()->is_symptomatic()) {
    #pragma omp atomic
    new_symptomatic_infections[disease_id]++;
    #pragma omp atomic
    total_symptomatic_infections[disease_id]++;
  }
}

void Place::report(int day) {}

void Place::print(int disease_id) {
  printf("Place %d label %s type %c\n", id, label, type);
  fflush(stdout);
}

void Place::enroll(Person * per) {
  enrollees.push_back(per);
  N++;
}

void Place::unenroll(Person * per) {
  N--;
}

void Place::add_susceptible(int disease_id, Person * per) {
  place_state[ disease_id ]().add_susceptible( per );
}

void Place::add_infectious(int disease_id, Person * per) {
  place_state[ disease_id ]().add_infectious( per );
  
  if ( !( infectious_bitset.test( disease_id ) ) ) {
    Disease * dis = population->get_disease( disease_id );
    dis->add_infectious_place( this, type );
    infectious_bitset.set( disease_id );
  }

  #pragma omp atomic
  current_infections[disease_id]++;

  if (per->get_health()->is_symptomatic()) {
    #pragma omp atomic
    current_symptomatic_infections[disease_id]++;
  }
}


void Place::print_susceptibles(int disease_id) {
 
  Place_State_Merge place_state_merge = Place_State_Merge();

  place_state[ disease_id ].apply( place_state_merge );

  std::vector< Person * > susceptibles = place_state_merge.get_susceptible_vector();

  vector<Person *>::iterator itr;
  for (itr = susceptibles.begin();
       itr != susceptibles.end(); itr++) {
    printf(" %d", (*itr)->get_id());
  }
  printf("\n");
}

void Place::print_infectious(int disease_id) {
  //vector<Person *>::iterator itr;
  //for (itr = infectious[disease_id].begin();
  //     itr != infectious[disease_id].end(); itr++) {
  //  printf(" %d", (*itr)->get_id());
  //}
  //printf("\n");
}

bool Place::is_open(int day) {
  if (container) {
    return container->is_open(day);
  } else {
    return (day < close_date || open_date <= day);
  }
}

double Place::get_contact_rate(int day, int disease_id) {
  
  Disease * disease = population->get_disease(disease_id);
  // expected number of susceptible contacts for each infectious person
  // OLD: double contacts = get_contacts_per_day(disease_id) * ((double) susceptibles[disease_id].size()) / ((double) (N-1));
  double contacts = get_contacts_per_day(disease_id) * disease->get_transmissibility();
  if (Global::Enable_Seasonality) {

    //contacts = contacts * Global::Clim->get_seasonality_multiplier_by_lat_lon(
    //    latitude,longitude,disease_id);

    double m = Global::Clim->get_seasonality_multiplier_by_lat_lon(
        latitude,longitude,disease_id);
    //cout << "SEASONALITY: " << day << " " << m << endl;
    contacts *= m;
  }

  // increase neighborhood contacts on weekends
  if (type == NEIGHBORHOOD) {
    int day_of_week = Global::Sim_Current_Date->get_day_of_week();
    if (day_of_week == 0 || day_of_week == 6) {
      contacts = Neighborhood::get_weekend_contact_rate(disease_id) * contacts;
    }
  }
  // FRED_VERBOSE(1,"Disease %d, expected contacts = %f\n", disease_id, contacts);
  return contacts;
}

int Place::get_contact_count(Person * infector, int disease_id, int day, double contact_rate) {
  // reduce number of infective contacts by infector's infectivity
  double infectivity = infector->get_infectivity(disease_id, day);
  double infector_contacts = contact_rate * infectivity;

  FRED_VERBOSE( 1, "infectivity = %f, so ", infectivity );
  FRED_VERBOSE( 1, "infector's effective contacts = %f\n", infector_contacts );
  
  // randomly round off the expected value of the contact counts
  int contact_count = (int) infector_contacts;
  double r = RANDOM();
  if (r < infector_contacts - contact_count) contact_count++;

  FRED_VERBOSE( 1, "infector contact_count = %d  r = %f\n", contact_count, r );
  
  return contact_count;
}

void Place::attempt_transmission(double transmission_prob, Person * infector, 
                                        Person * infectee, int disease_id, int day) {

  assert( infectee->is_susceptible( disease_id ) );
  FRED_STATUS(1,"infectee is susceptible\n","");
  
  double susceptibility = infectee->get_susceptibility(disease_id);
  FRED_VERBOSE( 2, "susceptibility = %f\n", susceptibility );

  double r = RANDOM();
  double infection_prob = transmission_prob * susceptibility;
  FRED_CONDITIONAL_VERBOSE( 1, r >= infection_prob,
      "transmission failed: r = %f  prob = %f\n", r, infection_prob );

  if (r < infection_prob) {
    // successful transmission; create a new infection in infectee
    Transmission transmission = Transmission(infector, this, day);
    infector->infect( infectee, disease_id, transmission );

    FRED_VERBOSE( 1, "transmission succeeded: r = %f  prob = %f\n", r, infection_prob );
    FRED_CONDITIONAL_VERBOSE( 1, infector->get_exposure_date(disease_id) == 0,
        "SEED infection day %i from %d to %d\n", day, infector->get_id(),infectee->get_id() );
    FRED_CONDITIONAL_VERBOSE( 1, infector->get_exposure_date(disease_id) != 0,
        "infection day %i of disease %i from %d to %d\n",
        day, disease_id, infector->get_id(), infectee->get_id() );
    FRED_CONDITIONAL_VERBOSE( 3, infection_prob > 1, "infection_prob exceeded unity!\n" );
  }
}

void Place::spread_infection(int day, int disease_id) {
  // Place::spread_infection is used for all derived places except for Households

  if ( is_open( day ) == false ) return;
  if ( should_be_open( day, disease_id ) == false ) return;

  if (first_day_infectious == -1) first_day_infectious = day;
  last_day_infectious = day;

  Place_State_Merge place_state_merge = Place_State_Merge();
  place_state[ disease_id ].apply( place_state_merge );
  std::vector< Person * > & susceptibles = place_state_merge.get_susceptible_vector();
  std::vector< Person * > & infectious = place_state_merge.get_infectious_vector();
  // need at least one susceptible
  if ( susceptibles.size() == 0 ) { return; }
  // the number of possible infectees per infector is max of (N-1) and S[s]
  // where N is the capacity of this place and S[s] is the number of current susceptibles
  // visiting this place. S[s] might exceed N if we have some ad hoc visitors,
  // since N is estimated only at startup.
  int number_targets = ( N - 1 > susceptibles.size() ? N - 1 : susceptibles.size() );

  // contact_rate is contacts_per_day with weeked and seasonality modulation (if applicable)
  double contact_rate = get_contact_rate(day,disease_id);

  // randomize the order of the infectious list
  FYShuffle<Person *>( infectious );

  for ( int infector_pos = 0; infector_pos < infectious.size(); ++infector_pos ) {
    // infectious visitor
    Person * infector = infectious[ infector_pos ];
    assert( infector->get_health()->is_infectious( disease_id ) );
    
    // get the actual number of contacts to attempt to infect
    int contact_count = get_contact_count( infector, disease_id, day, contact_rate );
    
    std::map< int, int > sampling_map;
    // get a susceptible target for each contact resulting in infection
    for (int c = 0; c < contact_count; ++c) {
      // select a target infectee from among susceptibles with replacement
      int pos = IRAND( 0, number_targets - 1 );
      if ( pos < susceptibles.size() ) {
        if ( infector == susceptibles[ pos ] ) {
          if ( susceptibles.size() > 1 ) {
            --( c ); // redo
            continue;
          }
          else {
            break; // give up
          }
        }
        sampling_map[ pos ]++;
      }
    }

    std::map< int, int >::iterator i;
    for ( i = sampling_map.begin(); i != sampling_map.end(); ++i ) {
      int pos = (*i).first;
      int times_drawn = (*i).second;
      Person * infectee = susceptibles[ pos ];
      // get the transmission probs for this infector/infectee pair
      double transmission_prob = get_transmission_prob(disease_id, infector, infectee);
      for ( int draw = 0; draw < times_drawn; ++draw ) {
        // only proceed if person is susceptible
        if ( infectee->is_susceptible( disease_id ) ) {
          attempt_transmission( transmission_prob, infector, infectee, disease_id, day );
        }
      }
    } // end contact loop
  } // end infectious list loop
}

Place * Place::select_neighborhood(double community_prob, double community_distance, double local_prob) {
  return grid_cell->select_neighborhood(community_prob, community_distance, local_prob);
}

Place * Place::select_new_neighborhood(double community_prob, double community_distance, double local_prob, double random) {
  return grid_cell->select_new_neighborhood(community_prob, community_distance, local_prob, random);
}

void Place::turn_workers_into_teachers(Place *school) {
  int new_teachers = 0;
  for (int i = 0; i < (int) enrollees.size(); i++) {
    if (enrollees[i]->become_a_teacher(school)) new_teachers++;
    // printf("new teacher %d age %d moving from workplace %s to school %s\n",
    // enrollees[i]->get_id(), enrollees[i]->get_age(), label, school->get_label());
  }
  FRED_VERBOSE(0, "%d new teachers reassigned from workplace %s to school %s\n",
	 new_teachers, label, school->get_label());
  N = 0;
}


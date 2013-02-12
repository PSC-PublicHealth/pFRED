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
// File: Household.cc
//
#include <limits>

#include "Household.h"
#include "Global.h"
#include "Params.h"
#include "Person.h"
#include "Cell.h"
#include "Grid.h"
#include "Utils.h"
#include "Random.h"
#include "Transmission.h"
#include "Large_Cell.h"
#include "Large_Grid.h"

//Private static variables that will be set by parameter lookups
double * Household::Household_contacts_per_day;
double *** Household::Household_contact_prob;

//Private static variable to assure we only lookup parameters once
bool Household::Household_parameters_set = false;

Household::Household( const  char *lab, fred::geo lon,
    fred::geo lat, Place *container, Population* pop ) {
  type = HOUSEHOLD;
  setup(lab, lon, lat, container, pop);
  get_parameters(Global::Diseases);
  housemate.clear();
  adults = children = 0;
  N = 0; 
  group_quarters = false;
}

void Household::get_parameters(int diseases) {
  char param_str[80];

  if (Household::Household_parameters_set) return;
  Household::Household_contacts_per_day = new double [ diseases ];
  Household::Household_contact_prob = new double** [ diseases ];
  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "household_contacts[%d]", s);
    Params::get_param((char *) param_str, &Household::Household_contacts_per_day[s]);
    sprintf(param_str, "household_prob[%d]", s);
    n = Params::get_param_matrix(param_str, &Household::Household_contact_prob[s]);
    // verbose status...
    if (Global::Verbose > 1) {
      printf("\nHousehold_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Household::Household_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  Household::Household_parameters_set = true;
}

int Household::get_group(int disease, Person * per) {
  int age = per->get_age();
  if (age < Global::ADULT_AGE) { return 0; }
  else { return 1; }
}

double Household::get_transmission_prob(int disease, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(disease, i);
  int col = get_group(disease, s);
  double tr_pr = Household::Household_contact_prob[disease][row][col];
  return tr_pr;
}

double Household::get_contacts_per_day(int disease) {
  return Household::Household_contacts_per_day[disease];
}

void Household::enroll(Person * per) {
  int age = per->get_age();
  N++;
  if (age < Global::ADULT_AGE)
    children++;
  else {
    adults++;
  }
  if (Global::Verbose>1) {
    printf("Add person %d to household %d\n", per->get_id(), get_id()); fflush(stdout);
  }
  housemate.push_back(per);
  // for (int i = 0; i < N; i++)
  // printf("%d ", housemate[i]->get_id()); 
  // printf("\n"); fflush(stdout);
  if (N == 1) {
    grid_cell->add_occupied_house();
  }
}

void Household::unenroll(Person * per) {
  int age = per->get_age();
  if (Global::Verbose>1) {
    printf("Removing person %d age %d from household %d\n",
        per->get_id(), age, get_id()); fflush(stdout);
    for (int i = 0; i < N; i++)
      printf("%d ", housemate[i]->get_id()); 
    printf("\n"); fflush(stdout);
    fflush(stdout);
  }

  // erase from housemates
  vector <Person *>::iterator it;
  for (it = housemate.begin(); *it != per && it != housemate.end(); it++);
  if (*it == per) {
    housemate.erase(it);
    N--;
    if (Global::Verbose>1) {
      printf("Removed person %d from household %d\n", per->get_id(), get_id());
      for (int i = 0; i < N; i++)
        printf("%d ", housemate[i]->get_id()); 
      printf("\n\n"); fflush(stdout);
    }
    if (N == 0) { 
      Global::Cells->add_vacant_house(this);
      grid_cell->subtract_occupied_house();
    }
  }
  else {
    printf("Removing person %d age %d from household %d\n",
        per->get_id(), age, get_id()); fflush(stdout);
    printf("Household::unenroll -- Help! unenrolled person not found in housemate list\n");
    for (int i = 0; i < N; i++)
      printf("%d ", housemate[i]->get_id()); 
    printf("\n"); fflush(stdout);
    Utils::fred_abort("");
  }

  // unenroll from large cell as well
  Large_Cell *large_cell = Global::Large_Cells->get_grid_cell(latitude, longitude);
  if(large_cell != NULL) large_cell->unenroll(per);
}

void Household::record_profile() {
  // record the ages in sorted order
  ages.clear();
  for (int i = 0; i < N; i++)
    ages.push_back(housemate[i]->get_age()); 
  sort(ages.begin(), ages.end());

  // record the id's of the original members of the household
  ids.clear();
  for (int i = 0; i < N; i++)
    ids.push_back(housemate[i]->get_id()); 
}


void Household::spread_infection(int day, int disease_id) {

  //Place_State_Merge place_state_merge = Place_State_Merge();
  //place_state[ disease_id ].apply( place_state_merge );
  //std::vector< Person * > & susceptibles = place_state_merge.get_susceptible_vector();
  //std::vector< Person * > & infectious = place_state_merge.get_infectious_vector();
  // need at least one susceptible, return otherwise
  if ( housemate.size() == 1 ) return;

  double contact_prob = get_contact_rate( day, disease_id );

  // randomize the order of the infectious list
  if ( !is_group_quarters() ) { // <--------------------------------------------------------------- TODO temporary support for group quarters 
    FYShuffle<Person *>( housemate );
  }

  for ( int infector_pos = 0; infector_pos < housemate.size(); ++infector_pos ) {
    Person * infector = housemate[ infector_pos ];      // infectious individual
    if ( ! infector->get_health()->is_infectious( disease_id ) ) { continue; }

    for (int pos = 0; pos < housemate.size(); ++pos) {
      if ( pos == infector_pos ) { continue; }
      if ( Global::Enable_Group_Quarters && is_group_quarters() ) { // <--------------------------- TODO temporary support for group quarters
        if ( gq_get_room_number( pos ) < gq_get_room_number( infector_pos ) ||
             gq_get_room_number( pos ) > gq_get_room_number( infector_pos ) ) {
          continue;
        }
      }
      Person * infectee = housemate[ pos ];
      // if a non-infectious person is selected, pick from non_infectious vector
      // only proceed if person is susceptible
      if ( infectee->is_susceptible( disease_id ) ) {
        // get the transmission probs for this infector/infectee pair
        double transmission_prob = get_transmission_prob( disease_id, infector, infectee );
        double infectivity = infector->get_infectivity( disease_id, day );
        // scale transmission prob by infectivity and contact prob
        transmission_prob *= infectivity * contact_prob;     
        attempt_transmission( transmission_prob, infector, infectee, disease_id, day );
      }
    } // end contact loop
  } // end infectious list loop
}

void Household::add_infectious(int disease_id, Person * per) {

  //place_state[ disease_id ]().add_infectious( per );
  
  if ( !( infectious_bitset.test( disease_id ) ) ) {
    Disease * dis = population->get_disease( disease_id );
    dis->add_infectious_place( this, type );
    infectious_bitset.set( disease_id );
  }

  if (per->get_health()->is_symptomatic()) {
    #pragma omp atomic
    Sympt[disease_id]++;
    #pragma omp atomic
    cases[disease_id]++;
    #pragma omp atomic
    total_cases[disease_id]++;
  }
}

int Household::gq_get_num_rooms() {
  return housemate.size() / gq_get_room_size();
}

int Household::gq_get_room_number( int housemate_index ) {
  return housemate_index / gq_get_room_size();
}








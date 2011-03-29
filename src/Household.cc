/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Household.cc
//

#include "Household.h"
#include "Global.h"
#include "Params.h"
#include "Person.h"
#include "Patch.h"

double * Household_contacts_per_day;
double *** Household_contact_prob;
int Household_parameters_set = 0;

Household::Household(int loc, const  char *lab, double lon, double lat, Place *container, Population* pop) {
  type = HOUSEHOLD;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_diseases());
  housemate.clear();
  adults = children = 0;
}

void Household::get_parameters(int diseases) {
  char param_str[80];
  
  if (Household_parameters_set) return;
  
  Household_contacts_per_day = new double [ diseases ];
  Household_contact_prob = new double** [ diseases ];
  
  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "household_contacts[%d]", s);
    get_param((char *) param_str, &Household_contacts_per_day[s]);
    
    sprintf(param_str, "household_prob[%d]", s);
    n = get_param_matrix(param_str, &Household_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nHousehold_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Household_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }

  Household_parameters_set = 1;
}

int Household::get_group(int disease, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Household::get_transmission_prob(int disease, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(disease, i);
  int col = get_group(disease, s);
  double tr_pr = Household_contact_prob[disease][row][col];
  return tr_pr;
}

double Household::get_contacts_per_day(int disease) {
  return Household_contacts_per_day[disease];
}

void Household::add_person(Person * per) {
  N++;
  if (per->get_age() < 18)
    children++;
  else {
    adults++;
    if (adults == 1) HoH = per;
  }
  // printf("Add person %d to household %d\n", per->get_id(), get_id()); fflush(stdout);
  housemate.push_back(per);
  // for (int i = 0; i < N; i++) printf("%d ", housemate[i]->get_id()); printf("\n"); fflush(stdout);
}

void Household::clear_counts() {
  N = 0; 
  adults = children = 0;
  housemate.clear();
}

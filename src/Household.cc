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

//Private static variables that will be set by parameter lookups
double * Household::Household_contacts_per_day;
double *** Household::Household_contact_prob;

//Private static variable to assure we only lookup parameters once
bool Household::Household_parameters_set = false;

Household::Household(int loc, const  char *lab, double lon,
		     double lat, Place *container, Population* pop) {
  type = HOUSEHOLD;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_diseases());
  housemate.clear();
  adults = children = 0;
  N = 0; 
}

void Household::get_parameters(int diseases) {
  char param_str[80];
  
  if (Household::Household_parameters_set) return;
  Household::Household_contacts_per_day = new double [ diseases ];
  Household::Household_contact_prob = new double** [ diseases ];
  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "household_contacts[%d]", s);
    get_param((char *) param_str, &Household::Household_contacts_per_day[s]);
    sprintf(param_str, "household_prob[%d]", s);
    n = get_param_matrix(param_str, &Household::Household_contact_prob[s]);
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
  N++;
  if (per->get_age() < Global::ADULT_AGE)
    children++;
  else {
    adults++;
    if (adults == 1) HoH = per;
  }
  housemate.push_back(per);
}

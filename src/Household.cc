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
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Household_contacts_per_day;
double *** Household_contact_prob;
int Household_parameters_set = 0;

Household::Household(int loc, char *lab, double lon, double lat, int container) {
  type = HOUSEHOLD;
  setup(loc, lab, lon, lat, container);
  get_parameters(Strains);
}

void Household::get_parameters(int strains) {
  char param_str[80];

  if (Household_parameters_set) return;

  Household_contacts_per_day = new double [ strains ];
  Household_contact_prob = new double** [ strains ];

  for (int s = 0; s < strains; s++) {
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

int Household::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Household::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Household_contact_prob[strain][row][col];
  return tr_pr;
}

double Household::get_contacts_per_day(int strain) {
  return Household_contacts_per_day[strain];
}



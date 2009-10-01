/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Household.cpp
//

#include "Household.hpp"

double * Household_contacts_per_day;
double *** Household_contact_prob;
int Household_parameters_set = 0;

Household::Household(int loc, char *lab, double lon, double lat) {
  type = HOUSEHOLD;
  setup(loc, lab, lon, lat);
  get_parameters();
}

void Household::get_parameters() {
  extern int Diseases;
  char param_str[80];

  if (Household_parameters_set) return;

  Household_contacts_per_day = new double [ Diseases ];
  Household_contact_prob = new double** [ Diseases ];

  for (int d = 0; d < Diseases; d++) {
    int n;
    sprintf(param_str, "household_contacts[%d]", d);
    get_param((char *) param_str, &Household_contacts_per_day[d]);

    sprintf(param_str, "household_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Household_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Household_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Household_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nHousehold_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Household_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Household_parameters_set = 1;
}

int Household::get_group_type(int dis, int per) {
  int age = Pop[per].get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Household::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Household_contact_prob[dis][row][col];
  return tr_pr;
}

double Household::get_contacts_per_day(int dis) {
  return Household_contacts_per_day[dis];
}





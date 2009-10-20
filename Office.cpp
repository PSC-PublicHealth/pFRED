/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Office.cpp
//

#include "Office.hpp"

double * Office_contacts_per_day;
double *** Office_contact_prob;
int Office_parameters_set = 0;

Office::Office(int loc, char *lab, double lon, double lat, int container) {
  type = OFFICE;
  setup(loc, lab, lon, lat, container);
  get_parameters(get_diseases());
}

void Office::get_parameters(int diseases) {
  char param_str[80];

  if (Office_parameters_set) return;

  Office_contacts_per_day = new double [ diseases ];
  Office_contact_prob = new double** [ diseases ];

  for (int d = 0; d < diseases; d++) {
    int n;
    sprintf(param_str, "office_contacts[%d]", d);
    get_param((char *) param_str, &Office_contacts_per_day[d]);

    sprintf(param_str, "office_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Office_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Office_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Office_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nOffice_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Office_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Office_parameters_set = 1;
}

int Office::get_group_type(int dis, int per) {
  return 0;
}

double Office::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Office_contact_prob[dis][row][col];
  return tr_pr;
}

double Office::get_contacts_per_day(int dis) {
  return Office_contacts_per_day[dis];
}





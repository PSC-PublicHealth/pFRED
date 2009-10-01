/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Hospital.cpp
//

#include "Hospital.hpp"

double * Hospital_contacts_per_day;
double *** Hospital_contact_prob;
int Hospital_parameters_set = 0;

Hospital::Hospital(int loc, char *lab, double lon, double lat) {
  type = HOSPITAL;
  setup(loc, lab, lon, lat);
  get_parameters();
}

void Hospital::get_parameters() {
  extern int Diseases;
  char param_str[80];

  if (Hospital_parameters_set) return;

  Hospital_contacts_per_day = new double [ Diseases ];
  Hospital_contact_prob = new double** [ Diseases ];

  for (int d = 0; d < Diseases; d++) {
    int n;
    sprintf(param_str, "hospital_contacts[%d]", d);
    get_param((char *) param_str, &Hospital_contacts_per_day[d]);

    sprintf(param_str, "hospital_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Hospital_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Hospital_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Hospital_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nHospital_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Hospital_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Hospital_parameters_set = 1;
}

int Hospital::get_group_type(int dis, int per) {
  int role = Pop[per].get_role(dis);
  if (role == PATIENT) { return 0; }
  else { return 1; }
}

double Hospital::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Hospital_contact_prob[dis][row][col];
  return tr_pr;
}

double Hospital::get_contacts_per_day(int dis) {
  return Hospital_contacts_per_day[dis];
}





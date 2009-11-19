/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Hospital.cc
//

#include "Hospital.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"
#include "Person.h"
#include "Behavior.h"

double * Hospital_contacts_per_day;
double *** Hospital_contact_prob;
int Hospital_parameters_set = 0;

Hospital::Hospital(int loc, char *lab, double lon, double lat, int container) {
  type = HOSPITAL;
  setup(loc, lab, lon, lat, container);
  get_parameters(Strains);
}

void Hospital::get_parameters(int strains) {
  char param_str[80];

  if (Hospital_parameters_set) return;

  Hospital_contacts_per_day = new double [ strains ];
  Hospital_contact_prob = new double** [ strains ];

  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "hospital_contacts[%d]", s);
    get_param((char *) param_str, &Hospital_contacts_per_day[s]);

    sprintf(param_str, "hospital_prob[%d]", s);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Hospital_contact_prob[s] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Hospital_contact_prob[s][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Hospital_contact_prob[s][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nHospital_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Hospital_contact_prob[s][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Hospital_parameters_set = 1;
}

int Hospital::get_group_type(int strain, Person * per) {
  return 0;
}

double Hospital::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Hospital_contact_prob[strain][row][col];
  return tr_pr;
}

double Hospital::get_contacts_per_day(int strain) {
  return Hospital_contacts_per_day[strain];
}





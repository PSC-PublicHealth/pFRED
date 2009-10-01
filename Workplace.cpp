/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Workplace.cpp
//

#include "Workplace.hpp"

double * Workplace_contacts_per_day;
double *** Workplace_contact_prob;
int Workplace_parameters_set = 0;

Workplace::Workplace(int loc, char *lab, double lon, double lat) {
  type = WORKPLACE;
  setup(loc, lab, lon, lat);
  get_parameters();
}

void Workplace::get_parameters() {
  extern int Diseases;
  char param_str[80];

  if (Workplace_parameters_set) return;

  Workplace_contacts_per_day = new double [ Diseases ];
  Workplace_contact_prob = new double** [ Diseases ];

  for (int d = 0; d < Diseases; d++) {
    int n;
    sprintf(param_str, "workplace_contacts[%d]", d);
    get_param((char *) param_str, &Workplace_contacts_per_day[d]);

    sprintf(param_str, "workplace_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Workplace_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Workplace_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Workplace_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nWorkplace_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Workplace_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Workplace_parameters_set = 1;
}

int Workplace::get_group_type(int dis, int per) {
  return 0;
}

double Workplace::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Workplace_contact_prob[dis][row][col];
  return tr_pr;
}

double Workplace::get_contacts_per_day(int dis) {
  return Workplace_contacts_per_day[dis];
}





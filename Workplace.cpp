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
#include "Global.hpp"
#include "Params.hpp"
#include "Random.hpp"
#include "Person.hpp"
#include "Strain.hpp"

double * Workplace_contacts_per_day;
double *** Workplace_contact_prob;
int Workplace_parameters_set = 0;

Workplace::Workplace(int loc, char *lab, double lon, double lat, int container) {
  type = WORKPLACE;
  setup(loc, lab, lon, lat, container);
  get_parameters(Strains);
}

void Workplace::get_parameters(int strains) {
  char param_str[80];

  if (Workplace_parameters_set) return;

  Workplace_contacts_per_day = new double [ strains ];
  Workplace_contact_prob = new double** [ strains ];

  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "workplace_contacts[%d]", s);
    get_param((char *) param_str, &Workplace_contacts_per_day[s]);

    sprintf(param_str, "workplace_prob[%d]", s);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Workplace_contact_prob[s] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Workplace_contact_prob[s][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Workplace_contact_prob[s][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nWorkplace_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Workplace_contact_prob[s][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Workplace_parameters_set = 1;
}

int Workplace::get_group_type(int strain, Person * per) {
  return 0;
}

double Workplace::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Workplace_contact_prob[strain][row][col];
  return tr_pr;
}

double Workplace::get_contacts_per_day(int strain) {
  return Workplace_contacts_per_day[strain];
}





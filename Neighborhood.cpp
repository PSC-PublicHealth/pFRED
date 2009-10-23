/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Neighborhood.cpp
//

#include "Neighborhood.hpp"

double * Neighborhood_contacts_per_day;
double *** Neighborhood_contact_prob;
int Neighborhood_parameters_set = 0;

Neighborhood::Neighborhood(int loc, char *lab, double lon,
			   double lat, int container) {
  type = NEIGHBORHOOD;
  setup(loc, lab, lon, lat, container);
  get_parameters(get_diseases());
}

void Neighborhood::get_parameters(int diseases) {
  char param_str[80];

  if (Neighborhood_parameters_set) return;

  Neighborhood_contacts_per_day = new double [ diseases ];
  Neighborhood_contact_prob = new double** [ diseases ];

  for (int d = 0; d < diseases; d++) {
    int n;
    sprintf(param_str, "neighborhood_contacts[%d]", d);
    get_param((char *) param_str, &Neighborhood_contacts_per_day[d]);
    // printf("Neighborhood_contacts_per_day[%d]= %f\n",
    // d,Neighborhood_contacts_per_day[d]);

    sprintf(param_str, "neighborhood_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    // printf("n = %d\n", n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      // printf("tmp:\n");
      // for (int i = 0; i < n; i++) printf("%f ", tmp[i]); printf("\n");

      n = (int) sqrt((double) n);
      // printf("\nNeighborhood_contact_prob:\n");
      Neighborhood_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Neighborhood_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Neighborhood_contact_prob[d][i][j] = tmp[i*n+j];
	  // printf("%f ",Neighborhood_contact_prob[d][i][j]);
	}
	// printf("\n");
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nNeighborhood_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Neighborhood_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }
  Neighborhood_parameters_set = 1;
}

int Neighborhood::get_group_type(int dis, int per) {
  int age = get_age(per);
  if (age < 18) { return 0; }
  else { return 1; }
}

double Neighborhood::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Neighborhood_contact_prob[dis][row][col];
  return tr_pr;
}

double Neighborhood::get_contacts_per_day(int dis) {
  return Neighborhood_contacts_per_day[dis];
}


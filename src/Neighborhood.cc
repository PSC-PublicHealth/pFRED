/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Neighborhood.cc
//

#include "Neighborhood.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Neighborhood_contacts_per_day;
double *** Neighborhood_contact_prob;
int Neighborhood_parameters_set = 0;

Neighborhood::Neighborhood(int loc, char *lab, double lon,
			   double lat, int container) {
  type = NEIGHBORHOOD;
  setup(loc, lab, lon, lat, container);
  get_parameters(Strains);
}

void Neighborhood::get_parameters(int strains) {
  char param_str[80];

  if (Neighborhood_parameters_set) return;

  Neighborhood_contacts_per_day = new double [ strains ];
  Neighborhood_contact_prob = new double** [ strains ];

  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "neighborhood_contacts[%d]", s);
    get_param((char *) param_str, &Neighborhood_contacts_per_day[s]);
    // printf("Neighborhood_contacts_per_day[%d]= %f\n",
    // s,Neighborhood_contacts_per_day[s]);

    sprintf(param_str, "neighborhood_prob[%d]", s);
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
      Neighborhood_contact_prob[s] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Neighborhood_contact_prob[s][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Neighborhood_contact_prob[s][i][j] = tmp[i*n+j];
	  // printf("%f ",Neighborhood_contact_prob[s][i][j]);
	}
	// printf("\n");
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nNeighborhood_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Neighborhood_contact_prob[s][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }
  Neighborhood_parameters_set = 1;
}

int Neighborhood::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Neighborhood::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Neighborhood_contact_prob[strain][row][col];
  return tr_pr;
}

double Neighborhood::get_contacts_per_day(int strain) {
  return Neighborhood_contacts_per_day[strain];
}



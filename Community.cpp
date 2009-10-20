/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Community.cpp
//

#include "Community.hpp"

double * Community_contacts_per_day;
double *** Community_contact_prob;
int Community_parameters_set = 0;

Community::Community(int loc, char *lab, double lon, double lat, int container) {
  type = COMMUNITY;
  setup(loc, lab, lon, lat, container);
  get_parameters(get_diseases());
}

void Community::get_parameters(int diseases) {
  char param_str[80];

  if (Community_parameters_set) return;

  Community_contacts_per_day = new double [ diseases ];
  Community_contact_prob = new double** [ diseases ];

  for (int d = 0; d < diseases; d++) {
    int n;
    sprintf(param_str, "community_contacts[%d]", d);
    get_param((char *) param_str, &Community_contacts_per_day[d]);
    // printf("Community_contacts_per_day[%d]= %f\n",d,Community_contacts_per_day[d]);

    sprintf(param_str, "community_prob[%d]", d);
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
      // printf("\nCommunity_contact_prob:\n");
      Community_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Community_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Community_contact_prob[d][i][j] = tmp[i*n+j];
	  // printf("%f ",Community_contact_prob[d][i][j]);
	}
	// printf("\n");
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nCommunity_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Community_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }
  Community_parameters_set = 1;
}

int Community::get_group_type(int dis, int per) {
  int age = get_age(per);
  if (age < 18) { return 0; }
  else { return 1; }
}

double Community::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Community_contact_prob[dis][row][col];
  return tr_pr;
}

double Community::get_contacts_per_day(int dis) {
  return Community_contacts_per_day[dis];
}



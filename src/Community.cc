/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Community.cc
//

#include "Community.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Community_contacts_per_day;
double *** Community_contact_prob;
int Community_parameters_set = 0;

Community::Community(int loc, const char *lab, double lon, double lat, Place *container, Population * pop) {
  type = COMMUNITY;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_strains());
}

void Community::get_parameters(int strains) {
  char param_str[80];
  
  if (Community_parameters_set) return;
  
  Community_contacts_per_day = new double [ strains ];
  Community_contact_prob = new double** [ strains ];
  
  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "community_contacts[%d]", s);
    get_param((char *) param_str, &Community_contacts_per_day[s]);
    // printf("Community_contacts_per_day[%d]= %f\n",d,Community_contacts_per_day[s]);
    sprintf(param_str, "community_prob[%d]", s);
    n = get_param_matrix(param_str, &Community_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nCommunity_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Community_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  Community_parameters_set = 1;
}

int Community::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Community::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Community_contact_prob[strain][row][col];
  return tr_pr;
}

double Community::get_contacts_per_day(int strain) {
  return Community_contacts_per_day[strain];
}
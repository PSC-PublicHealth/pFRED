/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Office.cc
//

#include "Office.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Office_contacts_per_day;
double *** Office_contact_prob;
int Office_parameters_set = 0;

Office::Office(int loc, const char *lab, double lon, double lat, Place *container, Population *pop) {
  type = OFFICE;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_strains());
}

void Office::get_parameters(int strains) {
  char param_str[80];
  
  if (Office_parameters_set) return;
  
  Office_contacts_per_day = new double [ strains ];
  Office_contact_prob = new double** [ strains ];
  
  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "office_contacts[%d]", s);
    get_param((char *) param_str, &Office_contacts_per_day[s]);
    
    sprintf(param_str, "office_prob[%d]", s);
    n = get_param_matrix(param_str, &Office_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nOffice_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Office_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  
  Office_parameters_set = 1;
}

int Office::get_group(int strain, Person * per) {
  return 0;
}

double Office::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(strain, i);
  int col = get_group(strain, s);
  double tr_pr = Office_contact_prob[strain][row][col];
  return tr_pr;
}

double Office::get_contacts_per_day(int strain) {
  return Office_contacts_per_day[strain];
}

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Classroom.cc
//

#include "Classroom.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Classroom_contacts_per_day;
double *** Classroom_contact_prob;
char Classroom_closure_policy[80];
int Classroom_closure_day;
double Classroom_closure_threshold;
int Classroom_closure_period;
int Classroom_closure_delay;
int Classroom_parameters_set = 0;


Classroom::Classroom(int loc, char *lab, double lon, double lat, int container) {
  type = CLASSROOM;
  setup(loc, lab, lon, lat, container);
  get_parameters(Strains);
}


void Classroom::get_parameters(int strains) {
  char param_str[80];

  if (Classroom_parameters_set) return;

  Classroom_contacts_per_day = new double [ strains ];
  Classroom_contact_prob = new double** [ strains ];

  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "classroom_contacts[%d]", s);
    get_param((char *) param_str, &Classroom_contacts_per_day[s]);

    sprintf(param_str, "classroom_prob[%d]", s);
    n = get_param_matrix(param_str, &Classroom_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nClassroom_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
	for (int j  = 0; j < n; j++) {
	  printf("%f ", Classroom_contact_prob[s][i][j]);
	}
	printf("\n");
      }
    }
  }

  Classroom_parameters_set = 1;
}

int Classroom::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < 19) { return 2; }
  else return 3;
}

double Classroom::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Classroom_contact_prob[strain][row][col];
  return tr_pr;
}

int Classroom::should_be_open(int day, int strain) {
  return Loc.location_should_be_open(container, day, strain);
}

double Classroom::get_contacts_per_day(int strain) {
  return Classroom_contacts_per_day[strain];
}

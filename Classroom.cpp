/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Classroom.cpp
//

#include "Classroom.hpp"

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
  get_parameters(get_diseases());
}


void Classroom::get_parameters(int diseases) {
  char param_str[80];

  if (Classroom_parameters_set) return;

  Classroom_contacts_per_day = new double [ diseases ];
  Classroom_contact_prob = new double** [ diseases ];

  for (int d = 0; d < diseases; d++) {
    int n;
    sprintf(param_str, "classroom_contacts[%d]", d);
    get_param((char *) param_str, &Classroom_contacts_per_day[d]);

    sprintf(param_str, "classroom_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      Classroom_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	Classroom_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  Classroom_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nClassroom_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", Classroom_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  Classroom_parameters_set = 1;
}

int Classroom::get_group_type(int dis, int per) {
  int age = get_age(per);
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < 19) { return 2; }
  else return 3;
}

double Classroom::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = Classroom_contact_prob[dis][row][col];
  return tr_pr;
}

int Classroom::should_be_open(int day, int dis) {
  return location_should_be_open(container, day, dis);
}

double Classroom::get_contacts_per_day(int dis) {
  return Classroom_contacts_per_day[dis];
}

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: School.cpp
//

#include "School.hpp"

double * School_contacts_per_day;
double *** School_contact_prob;
char School_closure_policy[80];
int School_closure_day;
double School_closure_threshold;
int School_closure_period;
int School_closure_delay;
int School_parameters_set = 0;


School::School(int loc, char *lab, double lon, double lat) {
  type = SCHOOL;
  setup(loc, lab, lon, lat);
  get_parameters();
}


void School::get_parameters() {
  extern int Diseases;
  char param_str[80];

  if (School_parameters_set) return;

  School_contacts_per_day = new double [ Diseases ];
  School_contact_prob = new double** [ Diseases ];

  for (int d = 0; d < Diseases; d++) {
    int n;
    sprintf(param_str, "school_contacts[%d]", d);
    get_param((char *) param_str, &School_contacts_per_day[d]);

    sprintf(param_str, "school_prob[%d]", d);
    n = 0;
    get_param((char *) param_str, &n);
    if (n) {
      double *tmp;
      tmp = new double [n];
      get_param_vector((char *) param_str, tmp);
      n = (int) sqrt((double) n);
      School_contact_prob[d] = new double * [n];
      for (int i  = 0; i < n; i++) 
	School_contact_prob[d][i] = new double [n];
      for (int i  = 0; i < n; i++) {
	for (int j  = 0; j < n; j++) {
	  School_contact_prob[d][i][j] = tmp[i*n+j];
	}
      }
      delete tmp;

      if (Verbose > 1) {
	printf("\nSchool_contact_prob:\n");
	for (int i  = 0; i < n; i++)  {
	  for (int j  = 0; j < n; j++) {
	    printf("%f ", School_contact_prob[d][i][j]);
	  }
	  printf("\n");
	}
      }
    }
  }

  get_param((char *) "school_closure_policy", School_closure_policy);
  get_param((char *) "school_closure_day", &School_closure_day);
  get_param((char *) "school_closure_threshold", &School_closure_threshold);
  get_param((char *) "school_closure_period", &School_closure_period);
  get_param((char *) "school_closure_delay", &School_closure_delay);

  School_parameters_set = 1;
}

int School::get_group_type(int dis, int per) {
  int age = Pop[per].get_age();
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < 19) { return 2; }
  else return 3;
}

double School::get_transmission_prob(int dis, int i, int s) {
  // dis = disease
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(dis, i);
  int col = get_group_type(dis, s);
  double tr_pr = School_contact_prob[dis][row][col];
  return tr_pr;
}

int School::should_be_open(int day, int dis) {
  extern double * Attack_rate;

  if (strcmp(School_closure_policy, "global") == 0) {
    //
    // Setting school_closure_day > -1 overrides other global triggers
    //
    // close schools if the closure date has arrived (after a delay)
    if (School_closure_day > -1 && School_closure_day == day) {
      close_date = day+School_closure_delay;
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }

    // Close schools if the global attack rate has reached the threshold
    // (with a delay)
    if (Attack_rate[dis] > School_closure_threshold) {
      if (is_open(day))
	close_date = day+School_closure_delay;
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }
  }

  if (strcmp(School_closure_policy, "reactive") == 0) {
    if (N == 0) return 0;
    double frac = (double) Sympt[dis] / (double) N;
    if (frac >= School_closure_threshold) {
      if (is_open(day)) {
	close_date = day+School_closure_delay;
      }
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }
  }

  // if school_closure_policy is not recognized, then open
  return 1;
}

double School::get_contacts_per_day(int dis) {
  return School_contacts_per_day[dis];
}


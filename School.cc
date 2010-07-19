/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: School.cc
//

#include "School.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * School_contacts_per_day;
double *** School_contact_prob;
char School_closure_policy[80];
int School_closure_day;
double School_closure_threshold;
int School_closure_period;
int School_closure_delay;
int School_parameters_set = 0;

School::School(int loc, const char *lab, double lon, double lat, Place* container, Population *pop) {
  type = SCHOOL;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_strains());
}


void School::get_parameters(int strains) {
  char param_str[80];
  
  if (School_parameters_set) return;
  
  School_contacts_per_day = new double [ strains ];
  School_contact_prob = new double** [ strains ];
  
  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "school_contacts[%d]", s);
    get_param((char *) param_str, &School_contacts_per_day[s]);
    
    sprintf(param_str, "school_prob[%d]", s);
    n = get_param_matrix(param_str, &School_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nSchool_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", School_contact_prob[s][i][j]);
        }
        printf("\n");
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

int School::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < 19) { return 2; }
  else return 3;
}

double School::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = School_contact_prob[strain][row][col];
  return tr_pr;
}

int School::should_be_open(int day, int strain) {
  
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
    Strain * str = Pop.get_strain(strain);
    if (str->get_attack_rate() > School_closure_threshold) {
      if (is_open(day))
        close_date = day+School_closure_delay;
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }
  }
  
  if (strcmp(School_closure_policy, "reactive") == 0) {
    if (N == 0) return 0;
    double frac = (double) Sympt[strain] / (double) N;
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

double School::get_contacts_per_day(int strain) {
  return School_contacts_per_day[strain];
}


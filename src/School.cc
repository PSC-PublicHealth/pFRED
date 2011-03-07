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
#include "Disease.h"

double * school_contacts_per_day;
double *** school_contact_prob;
char school_closure_policy[80];
int school_closure_day;
double school_closure_threshold;
int school_closure_period;
int school_closure_delay;
int school_parameters_set = 0;

School::School(int loc, const char *lab, double lon, double lat, Place* container, Population *pop) {
  type = SCHOOL;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_diseases());
}


void School::reset() {
  int diseases = population->get_diseases();
  for (int s = 0; s < diseases; s++) {
    susceptibles[s].clear();
    infectious[s].clear();
    Sympt[s] = S[s] = I[s] = 0;
    total_cases[s] = total_deaths[s] = 0;
  }
  if (Verbose > 2) {
    printf("reset place: %d\n", id);
    print(0);
    fflush(stdout);
  }
  close_date = INT_MAX;
  open_date = 0;
  for (int i = 0; i < 20; i++) grade[i] = 0;
}


void School::get_parameters(int diseases) {
  char param_str[80];
  
  if (school_parameters_set) return;
  
  school_contacts_per_day = new double [ diseases ];
  school_contact_prob = new double** [ diseases ];
  
  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "school_contacts[%d]", s);
    get_param((char *) param_str, &school_contacts_per_day[s]);
    
    sprintf(param_str, "school_prob[%d]", s);
    n = get_param_matrix(param_str, &school_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nschool_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", school_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  
  get_param((char *) "school_closure_policy", school_closure_policy);
  get_param((char *) "school_closure_day", &school_closure_day);
  get_param((char *) "school_closure_threshold", &school_closure_threshold);
  get_param((char *) "school_closure_period", &school_closure_period);
  get_param((char *) "school_closure_delay", &school_closure_delay);
 
  school_parameters_set = 1;
}

int School::get_group(int disease, Person * per) {
  int age = per->get_age();
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < 19) { return 2; }
  else return 3;
}

double School::get_transmission_prob(int disease, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(disease, i);
  int col = get_group(disease, s);
  double tr_pr = school_contact_prob[disease][row][col];
  return tr_pr;
}

int School::should_be_open(int day, int disease) {
  
  if (strcmp(school_closure_policy, "global") == 0) {
    //
    // Setting school_closure_day > -1 overrides other global triggers
    //
    // close schools if the closure date has arrived (after a delay)
    if (school_closure_day > -1 && school_closure_day == day) {
      close_date = day+school_closure_delay;
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
    
    // Close schools if the global attack rate has reached the threshold
    // (with a delay)
    Disease * str = Pop.get_disease(disease);
    if (str->get_attack_rate() > school_closure_threshold) {
      if (is_open(day))
        close_date = day+school_closure_delay;
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
  }
  
  if (strcmp(school_closure_policy, "individual") == 0) {
    if (N == 0) return 0;
    double frac = (double) Sympt[disease] / (double) N;
    if (frac >= school_closure_threshold) {
      if (is_open(day)) {
        close_date = day+school_closure_delay;
      }
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
  }
  
  // if school_closure_policy is not recognized, then open
  return 1;
}

double School::get_contacts_per_day(int disease) {
  return school_contacts_per_day[disease];
}

void School::add_person(Person * per) {
  N++;
  int age = per->get_age();
  if (age < 20) {
    grade[age]++;
  }
  else {
    grade[19]++;
  }
  
}



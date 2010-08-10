/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Health_Belief_Model.cc
//

#include "Health_Belief_Model.h"
#include "Person.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Spread.h"
#include "Perceptions.h"

int HBM_parameters_set = 0;

// Thresholds
double suscept_threshold[2];
double severity_threshold[2];
double benefits_threshold[2];
double barriers_threshold[2];
double susceptibility_threshold;

// Logistic approach -- see Durham 2010
double base_coeff_accept_vaccine;
double susc_coeff_accept_vaccine;
double sev_coeff_accept_vaccine;
double ben_coeff_accept_vaccine;
double bar_coeff_accept_vaccine;

double base_coeff_stay_home_if_sick;
double susc_coeff_stay_home_if_sick;
double sev_coeff_stay_home_if_sick;
double ben_coeff_stay_home_if_sick;
double bar_coeff_stay_home_if_sick;

// odd ratios
double base_OR_accept_vaccine;
double susc_OR_accept_vaccine;
double sev_OR_accept_vaccine;
double ben_OR_accept_vaccine;
double bar_OR_accept_vaccine;

// odd ratios
double base_OR_stay_home_if_sick;
double susc_OR_stay_home_if_sick;
double sev_OR_stay_home_if_sick;
double ben_OR_stay_home_if_sick;
double bar_OR_stay_home_if_sick;



Health_Belief_Model::Health_Belief_Model(Person *p) {
  self = p;
  // int strains = p->get_population()->get_strains();
  int strains = 1;

  perceptions = new Perceptions(p);
  get_parameters();
  memory_strength = 0.9;

  susceptibility = new (nothrow) int [strains];
  if (susceptibility == NULL) {
    printf("Help! susceptibility allocation failure\n");
    abort();
  }
  
  severity = new (nothrow) int [strains];
  if (severity == NULL) {
    printf("Help! severity allocation failure\n");
    abort();
  }
  
  benefits_stay_home_if_sick = new (nothrow) double [strains];
  if (benefits_stay_home_if_sick == NULL) {
    printf("Help! benefits allocation failure\n");
    abort();
  }
  
  barriers_stay_home_if_sick = new (nothrow) double [strains];
  if (barriers_stay_home_if_sick == NULL) {
    printf("Help! barrier allocation failure\n");
    abort();
  }
  benefits_accept_vaccine = new (nothrow) double [strains];
  if (benefits_accept_vaccine == NULL) {
    printf("Help! benefits allocation failure\n");
    abort();
  }
  
  barriers_accept_vaccine = new (nothrow) double [strains];
  if (barriers_accept_vaccine == NULL) {
    printf("Help! barrier allocation failure\n");
    abort();
  }

  memory_susceptibility = new (nothrow) double [strains];
  if (memory_susceptibility == NULL) {
    printf("Help! memory_susc allocation failure\n");
    abort();
  }

  memory_severity = new (nothrow) double [strains];
  if (memory_severity == NULL) {
    printf("Help! memory_severity allocation failure\n");
    abort();
  }
}

void Health_Belief_Model::get_parameters() {
  char s[80];
  double coeff[5];
  int n;
  // int strains = self->get_population()->get_strains();
  int strains = 1;

  if (HBM_parameters_set) return;

  Debug = 99;
  n = get_param_vector((char *) "HBM_stay_home_if_sick", coeff);
  if (n != 5) { printf("bad HBM_stay_home_if_sick"); abort(); }
  base_coeff_stay_home_if_sick = coeff[0];
  susc_coeff_stay_home_if_sick = coeff[1];
  sev_coeff_stay_home_if_sick = coeff[2];
  ben_coeff_stay_home_if_sick = coeff[3];
  bar_coeff_stay_home_if_sick = coeff[4];

  n = get_param_vector((char *) "HBM_accept_vaccine", coeff);
  if (n != 5) { printf("bad HBM_accept_vaccine"); abort(); }
  base_coeff_accept_vaccine = coeff[0];
  susc_coeff_accept_vaccine = coeff[1];
  sev_coeff_accept_vaccine = coeff[2];
  ben_coeff_accept_vaccine = coeff[3];
  bar_coeff_accept_vaccine = coeff[4];

  n = get_param_vector((char *) "HBM_suscept_threshold", suscept_threshold);
  if (n != 2) { printf("bad HBM_suscept_threshold\n"); abort(); }
  
  n = get_param_vector((char *) "HBM_severity_threshold", severity_threshold);
  if (n != 2) { printf("bad HBM_severity_threshold\n"); abort(); }
  
  n = get_param_vector((char *) "HBM_benefits_threshold", barriers_threshold);
  if (n != 2) { printf("bad HBM_benefits_threshold\n"); abort(); }
  
  n = get_param_vector((char *) "HBM_barriers_threshold", barriers_threshold);
  if (n != 2) { printf("bad HBM_barriers_threshold\n"); abort(); }
  
  susceptibility_threshold = suscept_threshold[0];
  HBM_parameters_set = 1;
}

void Health_Belief_Model::reset() {
  // int strains = self->get_population()->get_strains();
  int strains = 1;
  for (int s = 0; s < strains; s++) {
    memory_susceptibility[s] = 0.0;
    memory_severity[s] = 0.0;
  }
  stay_home_if_sick = false;
  accept_vaccine = false;
  avoid_crowds = false;
  keep_kids_home = false;
  wear_face_mask = false;
  total_deaths = 0;
}

void Health_Belief_Model::update(int day) {
  // int strains = self->get_population()->get_strains();
  int strains = 1;
  for (int s = 0; s < strains; s++) {
    // perceptions of current state of epidemic
    int current_cases = perceptions->get_global_cases(s);
    int total_cases = self->get_population()->get_strain(s)->get_spread()->get_total_incidents();
    double current_deaths = self->get_population()->get_strain(s)->get_mortality_rate()*total_cases;
    total_deaths += current_deaths;
    
    // update memory
    memory_susceptibility[s] = memory_strength * memory_susceptibility[s] + current_cases;
    memory_severity[s] = memory_strength * memory_severity[s] + current_deaths;

    // update HBM constructs
    
    // perceived susceptibility
    if (susceptibility_threshold <= memory_susceptibility[s])
      susceptibility[s] = 1;
    else
      susceptibility[s] = 0;
    
    // perceived severity
    if (total_cases > 0 && severity_threshold[s] <= total_deaths/total_cases)
      severity[s] = 1;
    else
      severity[s] = 0;
  }
  stay_home_if_sick = decide_whether_to_stay_home_if_sick();
  accept_vaccine = decide_whether_to_accept_vaccine();
  avoid_crowds = decide_whether_to_avoid_crowds();
  keep_kids_home = decide_whether_to_keep_kids_home();
  wear_face_mask = decide_whether_to_wear_face_mask();
}

bool Health_Belief_Model::decide_whether_to_stay_home_if_sick() {
  // return (RANDOM() < self->get_population()->get_strain(0)->get_prob_stay_home());
  return (RANDOM() < base_coeff_stay_home_if_sick);
}

bool Health_Belief_Model::decide_whether_to_accept_vaccine() {
  return false;
}

bool Health_Belief_Model::decide_whether_to_avoid_crowds() {
  return false;
}

bool Health_Belief_Model::decide_whether_to_keep_kids_home() {
  return false;
}

bool Health_Belief_Model::decide_whether_to_wear_face_mask() {
  return false;
}





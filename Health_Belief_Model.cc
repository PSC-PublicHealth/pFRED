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

// Thresholds for dichotomous variables
double susceptibility_threshold_distr[2];
double severity_threshold_distr[2];
double benefits_threshold_distr[2];
double barriers_threshold_distr[2];
double susceptibility_threshold;
double severity_threshold;
double benefits_threshold;
double barriers_threshold;

// Dichotomous logistic odd-ratios -- see Durham 2010
double base_accept_vaccine = 0.35;
double susceptibility_accept_vaccine = 3.0;
double severity_accept_vaccine = 1.93;
double benefits_accept_vaccine = 1.0;
double barriers_accept_vaccine = 0.65;

// memory decay factors
double memory_momentum_lower;
double memory_momentum_upper;

// population stats
double pop_cumm_susceptibility;
double pop_susceptibility;

Health_Belief_Model::Health_Belief_Model(Person *p) {
  self = p;
  int strains = p->get_population()->get_strains();

  perceptions = new Perceptions(p);
  get_parameters();
  memory_momentum = URAND(memory_momentum_lower,memory_momentum_upper);

  perceived_susceptibility = new (nothrow) int [strains];
  if (perceived_susceptibility == NULL) {
    printf("Help! sus allocation failure\n");
    abort();
  }
  
  perceived_severity = new (nothrow) int [strains];
  if (perceived_severity == NULL) {
    printf("Help! sev allocation failure\n");
    abort();
  }
  
  perceived_benefits_accept_vaccine = new (nothrow) double [strains];
  if (perceived_benefits_accept_vaccine == NULL) {
    printf("Help! benefits allocation failure\n");
    abort();
  }
  
  perceived_barriers_accept_vaccine = new (nothrow) double [strains];
  if (perceived_barriers_accept_vaccine == NULL) {
    printf("Help! barrier allocation failure\n");
    abort();
  }

  cumm_susceptibility = new (nothrow) double [strains];
  if (cumm_susceptibility == NULL) {
    printf("Help! cumm_susceptibility allocation failure\n");
    abort();
  }

  cumm_severity = new (nothrow) double [strains];
  if (cumm_severity == NULL) {
    printf("Help! cumm_severity allocation failure\n");
    abort();
  }

  accept_vaccine = new (nothrow) bool [strains];
  if (accept_vaccine == NULL) {
    printf("Help! accept_vaccine allocation failure\n");
    abort();
  }
}

void Health_Belief_Model::get_parameters() {
  double coeff[5];
  int n;

  if (HBM_parameters_set) return;

  get_param((char *) "HBM_memory_momentum_lower", &memory_momentum_lower);
  get_param((char *) "HBM_memory_momentum_upper", &memory_momentum_upper);

  n = get_param_vector((char *) "HBM_susceptibility_threshold", susceptibility_threshold_distr);
  if (n != 2) { printf("bad HBM_susceptibility_threshold\n"); abort(); }
  
  susceptibility_threshold = draw_normal(susceptibility_threshold_distr[0],susceptibility_threshold_distr[1]);

  n = get_param_vector((char *) "HBM_severity_threshold", severity_threshold_distr);
  if (n != 2) { printf("bad HBM_severity_threshold\n"); abort(); }
  
  severity_threshold = draw_normal(severity_threshold_distr[0],severity_threshold_distr[1]);

  n = get_param_vector((char *) "HBM_benefits_threshold", benefits_threshold_distr);
  if (n != 2) { printf("bad HBM_benefits_threshold\n"); abort(); }
  
  benefits_threshold = draw_normal(benefits_threshold_distr[0],benefits_threshold_distr[1]);

  n = get_param_vector((char *) "HBM_barriers_threshold", barriers_threshold_distr);
  if (n != 2) { printf("bad HBM_barriers_threshold\n"); abort(); }
  
  barriers_threshold = draw_normal(barriers_threshold_distr[0],barriers_threshold_distr[1]);

  n = get_param_vector((char *) "HBM_accept_vaccine", coeff);
  if (n != 5) { printf("bad HBM_accept_vaccine"); abort(); }
  base_accept_vaccine = coeff[0];
  susceptibility_accept_vaccine = coeff[1];
  severity_accept_vaccine = coeff[2];
  benefits_accept_vaccine = coeff[3];
  barriers_accept_vaccine = coeff[4];

  HBM_parameters_set = 1;
}

void Health_Belief_Model::reset() {
  int strains = self->get_population()->get_strains();
  for (int s = 0; s < strains; s++) {
    cumm_susceptibility[s] = 0.0;
    cumm_severity[s] = 0.0;
    accept_vaccine[s] = false;
  }
  total_deaths = 0;
  pop_cumm_susceptibility = 0.0;
  pop_susceptibility = 0.0;
  if (self->get_id() == 0) {
    FILE *fp;
    fp = fopen("HBM_trace", "w");
    fclose(fp);
  }
}

void Health_Belief_Model::update(int day) {
  int strains = self->get_population()->get_strains();
  for (int s = 0; s < strains; s++) {

    // perceptions of current state of epidemic
    int current_cases = perceptions->get_global_cases(s);
    int total_cases = self->get_population()->get_strain(s)->get_spread()->get_total_incidents();
    double current_deaths = self->get_population()->get_strain(s)->get_mortality_rate()*total_cases;
    double current_incidence = (double) current_cases / (double) self->get_population()->get_pop_size();
    total_deaths += current_deaths;
    
    // update memory
    cumm_susceptibility[s] = memory_momentum * cumm_susceptibility[s] + current_incidence;
    cumm_severity[s] = memory_momentum * cumm_severity[s] + current_deaths;

    // update HBM constructs
    
    // perceived susibility
    if (susceptibility_threshold <= cumm_susceptibility[s])
      perceived_susceptibility[s] = 1;
    else
      perceived_susceptibility[s] = 0;
    
    // perceived severity
    if (total_cases > 0 && severity_threshold <= total_deaths/total_cases)
      perceived_severity[s] = 1;
    else
      perceived_severity[s] = 0;
    accept_vaccine[s] = decide_whether_to_accept_vaccine(s);
  }
  if (self->get_id() == 0 && day > 0) {
    FILE *fp;
    int n = self->get_population()->get_pop_size();
    fp = fopen("HBM_trace", "a");
    fprintf(fp, "%d %f %d\n", day, cumm_susceptibility[0], accept_vaccine[0]);
    fprintf(fp, "%d %f %d\n", day+1, cumm_susceptibility[0], accept_vaccine[0]);
    // fprintf(fp, "%f %f\n", pop_cumm_susceptibility/n, ((double)pop_susceptibility)/n);
    fclose(fp);
    pop_cumm_susceptibility = 0.0;
    pop_susceptibility = 0.0;
  }
  pop_cumm_susceptibility += cumm_susceptibility[0];
  pop_susceptibility += perceived_susceptibility[0];
}

bool Health_Belief_Model::decide_whether_to_accept_vaccine(int s) {
  double Odds;
  Odds = base_accept_vaccine;
  if (perceived_susceptibility[s] == 1)
    Odds *= susceptibility_accept_vaccine;
  if (perceived_severity[s] == 1)
    Odds *= severity_accept_vaccine;
  if (perceived_benefits_accept_vaccine[s] == 1)
    Odds *= benefits_accept_vaccine;
  if (perceived_barriers_accept_vaccine[s] == 1)
    Odds *= barriers_accept_vaccine;
  return (Odds > 1.0);
}


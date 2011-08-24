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
#include "Epidemic.h"
#include "Perceptions.h"
#include "Utils.h"

int HBM_parameters_set = 0;

// Thresholds for dichotomous variables
double susceptibility_threshold_distr[2];
double severity_threshold_distr[2];
double benefits_threshold_distr[2];
double barriers_threshold_distr[2];
double memory_decay_distr[2];

// Dichotomous logistic odd-ratios -- see Durham 2010
double base_accept_vaccine = 0.35;
double susceptibility_accept_vaccine = 3.0;
double severity_accept_vaccine = 1.93;
double benefits_accept_vaccine = 1.0;
double barriers_accept_vaccine = 0.65;

// population stats
double pop_cumm_susceptibility;
double pop_susceptibility;

Health_Belief_Model::Health_Belief_Model(Person *p) {
  self = p;
  int diseases = Global::Pop.get_diseases();

  perceptions = new Perceptions(p);
  get_parameters();

  perceived_susceptibility = new (nothrow) int [diseases];
  if (perceived_susceptibility == NULL) {
    Utils::fred_abort("Help! sus allocation failure\n");
  }
  
  perceived_severity = new (nothrow) int [diseases];
  if (perceived_severity == NULL) {
    Utils::fred_abort("Help! sev allocation failure\n"); 
  }
  
  perceived_benefits_accept_vaccine = new (nothrow) double [diseases];
  if (perceived_benefits_accept_vaccine == NULL) {
    Utils::fred_abort("Help! benefits allocation failure\n"); 
  }
  
  perceived_barriers_accept_vaccine = new (nothrow) double [diseases];
  if (perceived_barriers_accept_vaccine == NULL) {
    Utils::fred_abort("Help! barrier allocation failure\n"); 
  }

  cumm_susceptibility = new (nothrow) double [diseases];
  if (cumm_susceptibility == NULL) {
    Utils::fred_abort("Help! cumm_susceptibility allocation failure\n"); 
  }

  cumm_severity = new (nothrow) double [diseases];
  if (cumm_severity == NULL) {
    Utils::fred_abort("Help! cumm_severity allocation failure\n"); 
  }

  accept_vaccine = new (nothrow) bool [diseases];
  if (accept_vaccine == NULL) {
    Utils::fred_abort("Help! accept_vaccine allocation failure\n");
  }

  // individual differences:
  memory_decay = draw_normal(memory_decay_distr[0],memory_decay_distr[1]);
  if (memory_decay < 0.001) memory_decay = 0.0001;

  susceptibility_threshold = draw_normal(susceptibility_threshold_distr[0],susceptibility_threshold_distr[1]);
  severity_threshold = draw_normal(severity_threshold_distr[0],severity_threshold_distr[1]);
  benefits_threshold = draw_normal(benefits_threshold_distr[0],benefits_threshold_distr[1]);
  barriers_threshold = draw_normal(barriers_threshold_distr[0],barriers_threshold_distr[1]);

  // sentinel agents:
  if (self->get_id() == 0) {
    memory_decay = memory_decay_distr[0] + memory_decay_distr[1];
    susceptibility_threshold = susceptibility_threshold_distr[0] + susceptibility_threshold_distr[1];
  }
  if (self->get_id() == 1) {
    memory_decay = memory_decay_distr[0] - memory_decay_distr[1];
    susceptibility_threshold = susceptibility_threshold_distr[0] - susceptibility_threshold_distr[1];
  }

  for (int s = 0; s < diseases; s++) {
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

void Health_Belief_Model::get_parameters() {
  double coeff[5];
  int n;

  if (HBM_parameters_set) return;

  n = Params::get_param_vector((char *) "HBM_memory_decay", memory_decay_distr);
  if (n != 2) {  
     Utils::fred_abort("bad HBM_memory_decay\n");  
  }
  
  n = Params::get_param_vector((char *) "HBM_susceptibility_threshold", susceptibility_threshold_distr);
  if (n != 2) { 
     Utils::fred_abort("bad HBM_susceptibility_threshold\n"); 
  }
  
  n = Params::get_param_vector((char *) "HBM_severity_threshold", severity_threshold_distr);
  if (n != 2) {  
     Utils::fred_abort("bad HBM_severity_threshold\n"); 
  }
  
  n = Params::get_param_vector((char *) "HBM_benefits_threshold", benefits_threshold_distr);
  if (n != 2) { 
     Utils::fred_abort("bad HBM_benefits_threshold\n"); 
  }
  
  n = Params::get_param_vector((char *) "HBM_barriers_threshold", barriers_threshold_distr);
  if (n != 2) {  
     Utils::fred_abort("bad HBM_barriers_threshold\n"); 
  }
  
  n = Params::get_param_vector((char *) "HBM_accept_vaccine", coeff);
  if (n != 5) {  
     Utils::fred_abort("bad HBM_accept_vaccine"); 
  }
  base_accept_vaccine = coeff[0];
  susceptibility_accept_vaccine = coeff[1];
  severity_accept_vaccine = coeff[2];
  benefits_accept_vaccine = coeff[3];
  barriers_accept_vaccine = coeff[4];

  HBM_parameters_set = 1;
}


void Health_Belief_Model::update(int day) {
  int diseases = Global::Pop.get_diseases();
  for (int s = 0; s < diseases; s++) {

    // perceptions of current state of epidemic
    int current_cases = perceptions->get_global_cases(s);
    int total_cases = Global::Pop.get_disease(s)->get_epidemic()->get_total_incidents();
    double current_deaths = Global::Pop.get_disease(s)->get_mortality_rate()*total_cases;
    double current_incidence = (double) current_cases / (double) Global::Pop.get_pop_size();
    total_deaths += current_deaths;
    
    // update memory
    if (day < 0) {
      cumm_susceptibility[s] = day*cumm_susceptibility[s] + current_incidence;
      cumm_susceptibility[s] /= day;
    }
    else {
      cumm_susceptibility[s] = (1.0 - memory_decay) * cumm_susceptibility[s] + memory_decay * current_incidence;
    }

    cumm_severity[s] = (1.0 - memory_decay) * cumm_severity[s] + memory_decay * current_deaths;

    // update HBM constructs
    
    // perceived susceptibility
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
    int n = Global::Pop.get_pop_size();
    fp = fopen("HBM_trace", "a");
    fprintf(fp, "%d %f %d ", day, cumm_susceptibility[0], accept_vaccine[0]);
    fprintf(fp, "%d %f %f ", day-1, pop_cumm_susceptibility/n, ((double)pop_susceptibility)/n);
    fclose(fp);
    pop_cumm_susceptibility = 0.0;
    pop_susceptibility = 0.0;
  }
  if (self->get_id() == 1 && day > 0) {
    FILE *fp;
    // (following variable 'n' not used)
    //    int n = self->get_population()->get_pop_size();
    fp = fopen("HBM_trace", "a");
    fprintf(fp, "%f %d\n", cumm_susceptibility[0], accept_vaccine[0]);
    fclose(fp);
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


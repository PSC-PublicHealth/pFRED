/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.cc
//

#include "Strain.h"

#include <stdio.h>
#include <new>
#include <string>
#include <sstream>
using namespace std;

#include "Global.h"
#include "Params.h"
#include "Population.h"
#include "Random.h"
#include "Age_Map.h"
#include "Epidemic.h"
#include "Timestep_Map.h"

double Prob_stay_home;

Strain::Strain() {
  transmissibility = -1.0;
  prob_symptomatic = -1.0;
  asymp_infectivity = -1.0;
  symp_infectivity = -1.0;
  max_days_latent = -1;
  max_days_incubating = -1;
  max_days_asymp = -1;
  max_days_symp = -1;
  max_days = 0;
  days_latent = NULL;
  days_incubating = NULL;
  days_asymp = NULL;
  days_symp = NULL;
  immunity_loss_rate = -1.0;
  mutation_prob = NULL;
  residual_immunity = NULL;
  at_risk = NULL;
  epidemic = NULL;
  population = NULL;
}

Strain::~Strain() {
  delete [] days_latent;
  delete [] days_incubating;
  delete [] days_asymp;
  delete [] days_symp;
  delete epidemic;
  delete residual_immunity;
  delete at_risk;
}

void Strain::reset() {
  epidemic->reset();
}

void Strain::setup(int strain, Population *pop, double *mut_prob) {
  char s[80];
  id = strain;
  int n;
  
  get_indexed_param("trans",id,&transmissibility);
  get_indexed_param("symp",id,&prob_symptomatic);
  get_indexed_param("mortality_rate",id,&mortality_rate);
  get_indexed_param("symp_infectivity",id,&symp_infectivity);
  get_indexed_param("asymp_infectivity",id,&asymp_infectivity);

  get_indexed_param("days_latent",id,&n);
  days_latent = new double [n];
  max_days_latent = get_indexed_param_vector("days_latent", id, days_latent) -1;
  
  get_indexed_param("days_incubating",id,&n);
  days_incubating = new double [n];
  max_days_incubating = get_indexed_param_vector("days_incubating",id, days_incubating) - 1;
    
  get_indexed_param("days_asymp",id,&n);
  days_asymp = new double [n];
  max_days_asymp = get_indexed_param_vector("days_asymp", id, days_asymp) -1;
  
  get_indexed_param("days_symp",id,&n);
  days_symp = new double [n];
  max_days_symp = get_indexed_param_vector("days_symp", id, days_symp) -1;
  
  get_indexed_param("immunity_loss_rate",id,&immunity_loss_rate);
  get_indexed_param("infection_model",id,&infection_model);
  
  // This needs to be moved to Cognition
  get_param((char *) "prob_stay_home", &Prob_stay_home);
  
  if (max_days_asymp > max_days_symp) {
    max_days = max_days_latent + max_days_asymp;
  }
  else {
    max_days = max_days_latent + max_days_symp;
  }

  mutation_prob = mut_prob;
  population = pop;
	
  // Read primary_cases file that indicates the number of externally infections
  // occur each day.
  // Note: infectees are chosen at random, and previously infected individuals
  // are not affected, so the number of new cases may be less than specified in
  // the file.
  char param_name[80];
  sprintf(param_name,"primary_cases[%d]",id);
  string param_name_str(param_name);
  epidemic = new Epidemic(this, new Timestep_Map(param_name_str));
  
  // Define residual immunity
  residual_immunity = new Age_Map("Residual Immunity");
  residual_immunity->read_from_input("residual_immunity",id);
  if(residual_immunity->is_empty() == false) residual_immunity->print();
  
  // Define at risk people
  at_risk = new Age_Map("At Risk Population");
  at_risk->read_from_input("at_risk",id);
  if(at_risk->is_empty() == false ) at_risk->print();	
	
  printf("Strain setup finished\n"); fflush(stdout);
  if (Verbose) print();
}

void Strain::print() {
  printf("strain %d symp %.3f trans %e symp_infectivity %.3f asymp_infectivity %.3f\n",
         id, prob_symptomatic, transmissibility, symp_infectivity, asymp_infectivity);
  printf("days latent: ");
  for (int i = 0; i <= max_days_latent; i++)
    printf("%.3f ", days_latent[i]);
  printf("\n");
  printf("days incubating: ");
  for (int i = 0; i <= max_days_incubating; i++)
    printf("%.3f ", days_incubating[i]);
  printf("\n");
  printf("days symp: ");
  for (int i = 0; i <= max_days_symp; i++)
    printf("%.3f ", days_symp[i]);
  printf("\n");
  printf("days asymp: ");
  for (int i = 0; i <= max_days_asymp; i++)
    printf("%.3f ", days_asymp[i]);
  printf("\n");
}

Strain* Strain::should_mutate_to() {
  int num_strains = population->get_strains();
  // Pick a random index to consider mutations from, so that mutating to all strains is
  // dependent only on muation_prob, and not on the order of the strains.
  int strain_start = IRAND(0, num_strains-1);
  // By default, create an infection with this strain.
  int infection_id = id;
  
  // Try and mutate to other strains.
  for (int strain_i = strain_start; strain_i < num_strains; ++strain_i) {
    if (strain_i == id) continue;
    double r = RANDOM();
    if (r < mutation_prob[strain_i]) {
      return population->get_strain(strain_i);
    }
  }
  if (infection_id == id) {
    // Didn't mutate yet.
    for (int strain_i = 0; strain_i < strain_start; ++strain_i) {
      if (strain_i == id) continue;
      double r = RANDOM();
      if (r < mutation_prob[strain_i]) {
        return population->get_strain(strain_i);
      }
    }
  }
  return NULL;
}

int Strain::get_days_latent() {
  int days = 0;
  days = draw_from_distribution(max_days_latent, days_latent);
  return days;
}

int Strain::get_days_incubating() {
  int days = 0;
  days = draw_from_distribution(max_days_incubating, days_incubating);
  return days;
}

int Strain::get_days_asymp() {
  int days = 0;
  days = draw_from_distribution(max_days_asymp, days_asymp);
  return days;
}

int Strain::get_days_symp() {
  int days = 0;
  days = draw_from_distribution(max_days_symp, days_symp);
  return days;
}

int Strain::get_days_recovered() {
  int days;
  if (immunity_loss_rate > 0.0) {
    // draw from exponential distribution
    days = floor(0.5 + draw_exponential(immunity_loss_rate));
  } else {
    days = -1;
  }
  return days;
}

int Strain::get_symptoms() {
  return (RANDOM() < prob_symptomatic);
}

double Strain::get_attack_rate() {
  return epidemic->get_attack_rate();
}

void Strain::update_stats(int day) {
  epidemic->update_stats(day);
}

void Strain::print_stats(int day) {
  epidemic->print_stats(day);
}

// static
double Strain::get_prob_stay_home() { return Prob_stay_home; }

// static
void Strain::set_prob_stay_home(double p) { Prob_stay_home = p; }

// static
void Strain::get_strain_parameters() {
}

void Strain::update(int day) {
  epidemic->update(day);
}

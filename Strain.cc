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
#include "Infection.h"
#include "Locations.h"
#include "Params.h"
#include "Person.h"
#include "Place.h"
#include "Population.h"
#include "Random.h"
#include "Age_Map.h"
#include "Spread.h"

double Prob_stay_home;

Strain::Strain() {
  residual_immunity = NULL;
}

void Strain::reset() {
  spread->reset();
}

void Strain::setup(int strain, Population *pop, double *mut_prob) {
  char s[80];
  id = strain;
  int n;

  sprintf(s, "trans[%d]", id);
  get_param(s, &transmissibility);

  sprintf(s, "symp[%d]", id);
  get_param(s, &prob_symptomatic);

  sprintf(s, "symp_infectivity[%d]", id);
  get_param(s, &symp_infectivity);

  sprintf(s, "asymp_infectivity[%d]", id);
  get_param(s, &asymp_infectivity);

  sprintf(s, "days_latent[%d]", id);
  get_param(s, &n);
  days_latent = new double [n];
  max_days_latent = get_param_vector(s, days_latent) -1;

  sprintf(s, "days_incubating[%d]", id);
  get_param(s, &n);
  days_incubating = new double [n];
  max_days_incubating = get_param_vector(s, days_incubating) -1;

  // This check disallows deprecated strain params
  sprintf(s, "days_infectious[%d]", id);
  if (does_param_exist(s)) {
    printf("***** Found deprecated parameter ***** %s\n"
	   "New parameters are days_asymp and days_symp "
	   "Aborting\n", s);
    abort();
  }

  sprintf(s, "days_asymp[%d]", id);
  get_param(s, &n);
  days_asymp = new double [n];
  max_days_asymp = get_param_vector(s, days_asymp) -1;

  sprintf(s, "days_symp[%d]", id);
  get_param(s, &n);
  days_symp = new double [n];
  max_days_symp = get_param_vector(s, days_symp) -1;

  sprintf(s, "immunity_loss_rate[%d]", id);
  get_param(s, &immunity_loss_rate);

  get_param((char *) "prob_stay_home", &Prob_stay_home);

  mutation_prob = mut_prob;
  population = pop;
  spread = new Spread(this);

  // Define residual immunity
  sprintf(s,"residual_immunity_ages[%d]",id);
  residual_immunity = new Age_Map("Residual Immunity");
  stringstream ss; 
  ss << "residual_immunity[" << id << "]";
  residual_immunity->read_from_input(ss.str());
  residual_immunity->print();
  
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

bool Strain::attempt_infection(Person* infector, Person* infectee,
			       Place* place, int exposure_date) {
  // is the victim here today, and still susceptible?
  if (infectee->is_on_schedule(exposure_date, place->get_id()) &&
      infectee->get_strain_status(id) == 'S') {
    if (Verbose > 1) { printf("Victim is here and is susceptible\n"); }

    // get the victim's susceptibility
    double susceptibility;
    double transmission_prob;
    if (place && infector) {
      transmission_prob = place->get_transmission_prob(id, infector, infectee);
      susceptibility = infectee->get_susceptibility(id);
    } else {
      // This isn't a regular transmission - it's a seed case or a mutation.
      transmission_prob = susceptibility = 1.0;
    }
    if (Verbose > 1) {
      printf("trans_prob = %f  susceptibility = %f\n",
	     transmission_prob, susceptibility);
    }

    double r = RANDOM();
    if (r < transmission_prob*susceptibility) {
      if (Verbose > 1) { printf("transmission succeeded: r = %f\n", r); }
      Infection* i = new Infection(this, infector, infectee, place, exposure_date);
      infectee->become_exposed(i);
      if (infector)
	infector->add_infectee(id);
      return true;
    }
    if (Verbose > 1) { printf("transmission failed: r = %f\n", r); }
    return false;
  }
  if (Verbose > 1) {
    printf("Victim either not here or not susceptible, status = %c  exp_date = %d place_id = %d\n",
	   infectee->get_strain_status(id), exposure_date, place->get_id());
  }
  return false;
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
  return spread->get_attack_rate();
}

void Strain::update_stats(int day) {
  spread->update_stats(day);
}

void Strain::print_stats(int day) {
  spread->print_stats(day);
}

// static
double Strain::get_prob_stay_home() { return Prob_stay_home; }

// static
void Strain::get_strain_parameters() {
  // get_param((char *) "strains", &Strains);
}

void Strain::insert_into_exposed_list(Person * per) {
  spread->insert_into_exposed_list(per);
  // exposed.insert(per);
}

void Strain::insert_into_infectious_list(Person * per) {
  spread->insert_into_infectious_list(per);
  // infectious.insert(per);
}

void Strain::remove_from_exposed_list(Person * per) {
  spread->remove_from_exposed_list(per);
  // exposed.erase(per);
}

void Strain::remove_from_infectious_list(Person * per) {
  spread->remove_from_infectious_list(per);
  // infectious.erase(per);
}

void Strain::update(int day) {
  spread->update(day);
}

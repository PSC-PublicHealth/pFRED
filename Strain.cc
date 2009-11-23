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
using namespace std;

#include "Global.h"
#include "Infection.h"
#include "Locations.h"
#include "Params.h"
#include "Person.h"
#include "Population.h"
#include "Random.h"
#include "Spread.h"

double Prob_stay_home;

Strain::Strain() {
}

void Strain::reset() {
  spread->reset();
}

void Strain::setup(int strain, Population *pop) {
  char s[80];
  id = strain;
  int n;

  sprintf(s, "trans[%d]", id);
  get_param(s, &transmissibility);

  sprintf(s, "symp[%d]", id);
  get_param(s, &prob_symptomatic);

  sprintf(s, "resistant[%d]", id);
  // get_param(s, &prob_resistant);

  sprintf(s, "days_latent[%d]", id);
  get_param(s, &n);
  days_latent = new double [n];
  max_days_latent = get_param_vector(s, days_latent) -1;

  sprintf(s, "days_incubating[%d]", id);
  get_param(s, &n);
  days_incubating = new double [n];
  max_days_incubating = get_param_vector(s, days_incubating) -1;

  sprintf(s, "days_infectious[%d]", id);
  get_param(s, &n);
  days_infectious = new double [n];
  max_days_infectious = get_param_vector(s, days_infectious) -1;
  printf("Strain setup finished\n"); fflush(stdout);

  get_param((char *) "prob_stay_home", &Prob_stay_home);
  spread = new Spread(this);
}

void Strain::print() {
  printf("strain %d symp %.3f resist %.3f trans %e\n",
	 id, prob_symptomatic, prob_resistant, transmissibility);
  printf("days latent: ");
  for (int i = 0; i <= max_days_latent; i++)
    printf("%.3f ", days_latent[i]);
  printf("\n");
  printf("days incubating: ");
  for (int i = 0; i <= max_days_incubating; i++)
    printf("%.3f ", days_incubating[i]);
  printf("\n");
  printf("days infectious: ");
  for (int i = 0; i <= max_days_infectious; i++)
    printf("%.3f ", days_infectious[i]);
  printf("\n");
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

int Strain::get_days_infectious() {
  int days = 0;
  days = draw_from_distribution(max_days_infectious, days_infectious);
  return days;
}


// static
int Strain::draw_from_distribution(int n, double *dist) {
  double r = RANDOM();
  int i = 0;
  while (i <= n && dist[i] < r) { i++; }
  if (i <= n) { return i; }
  else {
    printf("Help! draw from distribution failed.\n");
    printf("Is distribution properly formed? (should end with 1.0)\n");
    for (int i = 0; i <= n; i++) {
      printf("%f ", dist[i]);
    }
    printf("\n");
    return -1;
  }
}

int Strain::get_symptoms() {
  return (RANDOM() < prob_symptomatic);
}

int Strain::get_index_cases() {
  return spread->get_index_cases();
}

double Strain::get_attack_rate() {
  return spread->get_attack_rate();
}

void Strain::start_outbreak(Person *pop, int pop_size) {
  spread->start_outbreak(pop, pop_size);
}

void Strain::update_stats(Person *pop, int pop_size, int day) {
  spread->update_stats(pop, pop_size, day);
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

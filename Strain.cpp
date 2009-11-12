/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.cpp
//

#include "Strain.hpp"

#include <stdio.h>
#include <new>
using namespace std;

#include "Random.hpp"
#include "Params.hpp"

Strain *Str;
int Strains;
double Prob_stay_home;

void Strain::setup(int strain) {
  char s[80];
  id = strain;
  int n;

  sprintf(s, "trans[%d]", id);
  get_param(s, &transmissibility);

  sprintf(s, "symp[%d]", id);
  get_param(s, &prob_symptomatic);

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
}

void Strain::print() {
  printf("strain %d sym %.3f trans %e\n",
	 id, prob_symptomatic, transmissibility);
  printf("days latent: ");
  for (int i = 0; i <= max_days_latent; i++)
    printf("%.3f ", days_latent[i]);
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

int Strain::get_days_infectious() {
  int days = 0;
  days = draw_from_distribution(max_days_infectious, days_infectious);
  return days;
}


// static
void Strain::get_strain_parameters() {
  get_param((char *) "strains", &Strains);
  get_param((char *) "prob_stay_home", &Prob_stay_home);
}

// static
void Strain::setup_strains(int verbose) {
  Str = new (nothrow) Strain [Strains];
  if (Str == NULL) {
    printf("Help! Str allocation failure\n");
    abort();
  }
  for (int d = 0; d < Strains; d++) {
    Str[d].setup(d);
    if (verbose) Str[d].print();
  }
}

// static
int Strain::draw_from_distribution(int n, double *straint) {
  double r = RANDOM();
  int i = 0;
  while (i <= n && straint[i] < r) { i++; }
  if (i <= n) { return i; }
  else {
    printf("Help! draw from distribution failed.\n");
    printf("Is distribution properly formed? (should end with 1.0)\n");
    for (int i = 0; i <= n; i++) {
      printf("%f ", straint[i]);
    }
    printf("\n");
    return -1;
  }
}

// static
int Strain::get_strains() { return Strains; }

// static
double Strain::get_beta(int strain) { return Str[strain].get_transmissibility(); }

// static
double Strain::get_prob_symptomatic(int strain) { return Str[strain].get_prob_symptomatic(); }

// static
int Strain::get_days_latent(int strain) { return Str[strain].get_days_latent(); }

// static
int Strain::get_days_infectious(int strain) {  return Str[strain].get_days_infectious(); }

// static
double Strain::get_prob_stay_home() { return Prob_stay_home; }

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.cpp
//

#include "Disease.hpp"

#include <stdio.h>
#include <new>
using namespace std;

#include "Random.hpp"
#include "Params.hpp"

Disease *Dis;
int Diseases;
double Prob_stay_home;

void Disease::setup(int dis) {
  char s[80];
  id = dis;
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

void Disease::print() {
  printf("disease %d sym %.3f trans %e\n",
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

int Disease::get_days_latent() {
  int days = 0;
  days = draw_from_distribution(max_days_latent, days_latent);
  return days;
}

int Disease::get_days_infectious() {
  int days = 0;
  days = draw_from_distribution(max_days_infectious, days_infectious);
  return days;
}


// static
void Disease::get_disease_parameters() {
  get_param((char *) "diseases", &Diseases);
  get_param((char *) "prob_stay_home", &Prob_stay_home);
}

// static
void Disease::setup_diseases(int verbose) {
  Dis = new (nothrow) Disease [Diseases];
  if (Dis == NULL) {
    printf("Help! Dis allocation failure\n");
    abort();
  }
  for (int d = 0; d < Diseases; d++) {
    Dis[d].setup(d);
    if (verbose) Dis[d].print();
  }
}

// static
int Disease::draw_from_distribution(int n, double *dist) {
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

// static
int Disease::get_diseases() { return Diseases; }

// static
double Disease::get_beta(int dis) { return Dis[dis].get_transmissibility(); }

// static
double Disease::get_prob_symptomatic(int dis) { return Dis[dis].get_prob_symptomatic(); }

// static
int Disease::get_days_latent(int dis) { return Dis[dis].get_days_latent(); }

// static
int Disease::get_days_infectious(int dis) {  return Dis[dis].get_days_infectious(); }

// static
double Disease::get_prob_stay_home() { return Prob_stay_home; }

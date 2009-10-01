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

Disease *Dis;
int Diseases;


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



void get_disease_parameters() {
  get_param((char *) "diseases", &Diseases);
}

void setup_diseases() {
  extern int Verbose;
  Dis = new (nothrow) Disease [Diseases];
  if (Dis == NULL) {
    printf("Help! Dis allocation failure\n");
    abort();
  }
  for (int d = 0; d < Diseases; d++) {
    Dis[d].setup(d);
    if (Verbose) Dis[d].print();
  }
}

int draw_from_distribution(int n, double *dist) {
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


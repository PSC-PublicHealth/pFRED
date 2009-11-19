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
#include "Person.hpp"
#include "Global.hpp"
double Prob_stay_home;

Strain::Strain() {
  get_param((char *) "prob_stay_home", &Prob_stay_home);
  // get_param((char *) "index_cases", &index_cases);
}

void Strain::reset() {
  exposed.clear();
  infectious.clear();
  attack_rate = 0.0;
  S = E = I = R = 0;
}

void Strain::setup(int strain) {
  char s[80];
  id = strain;
  int n;

  sprintf(s, "trans[%d]", id);
  get_param(s, &transmissibility);

  sprintf(s, "symp[%d]", id);
  get_param(s, &prob_symptomatic);

  sprintf(s, "resistant[%d]", id);
  // get_param(s, &prob_resistant);

  sprintf(s, "index_cases[%d]", id);
  get_param(s, &index_cases);

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

void Strain::start_outbreak(Person *pop, int pop_size) {
  // create index cases
  for (int i = 0; i < index_cases; i++) {
    int n = IRAND(0, pop_size-1);
    pop[n].become_exposed(this, -1, -1, 'X', 0);
  }
  E = index_cases;
}

void Strain::update_exposed(int day) {
  Person * p;
  set<Person *>::iterator it;
  stack <Person *> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_exposed for strain %d day %d\n", id, day);
    fflush(Statusfp);
  }

  for (it = exposed.begin(); it != exposed.end(); it++) {
    p = *it;
    if (p->get_infectious_date(id) == day) {
      TempList.push(p);
    }
  }
  
  if (Verbose > 1) {
    fprintf(Statusfp,
	    "update_exposed: new infectious for day %d strain %d = %d\n",
	    day, id, (int) TempList.size());
    fflush(Statusfp);
  }
  
  while (!TempList.empty()) {
    p = TempList.top();
    p->become_infectious(this);
    TempList.pop();
  }

  if (Verbose > 1) {
    fprintf(Statusfp,
	    "update_exposed finished for day %d strain %d = %d\n",
	    day, id, (int) TempList.size());
    fflush(Statusfp);
  }
  
}

void Strain::update_infectious(int day) {
  Person * p;
  set<Person *>::iterator it;
  stack <Person *> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_infectious for strain %d day %d\n", id, day);
    fflush(Statusfp);
  }

  for (it = infectious.begin(); it != infectious.end(); it++) {
    p = *it;
    // printf("inf = %d recov = %d\n", p->get_id(), p->get_recovered_date(s));
    // fflush(stdout);
    if (p->get_recovered_date(id) == day) {
      TempList.push(p);
    }
  }

  // printf("Templist size = %d\n", (int) TempList.size()); fflush(stdout);
  while (!TempList.empty()) {
    p = TempList.top();
    // printf("top = %d\n", p->get_id()); fflush(stdout);
    p->recover(this);
    TempList.pop();
  }

  if (Verbose > 1) {
    fprintf(Statusfp, "update_infectious for day %d complete\n", day);
    fflush(Statusfp);
  }
}

void Strain::update_stats(Person *pop, int pop_size, int day) {
  S = E = I = R = 0;
  for (int p = 0; p < pop_size; p++) {
    char status = pop[p].get_strain_status(id);
    S += (status == 'S');
    E += (status == 'E');
    I += (status == 'I') || (status == 'i');
    R += (status == 'R');
  }
  attack_rate = (100.0*(E+I+R))/pop_size;
}

void Strain::print_stats(int day) {
  int N = S+E+I+R;
  fprintf(Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n",
	  day, id, S, E, I, R, N, attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n\n",
	    day, id, S, E, I, R, N, attack_rate);
    fflush(Statusfp);
  }
}

// static
double Strain::get_prob_stay_home() { return Prob_stay_home; }

// static
void Strain::get_strain_parameters() {
  // get_param((char *) "strains", &Strains);
}

void Strain::insert_into_exposed_list(Person * per) {
  exposed.insert(per);
}

void Strain::insert_into_infectious_list(Person * per) {
  infectious.insert(per);
}

void Strain::remove_from_exposed_list(Person * per) {
  exposed.erase(per);
}

void Strain::remove_from_infectious_list(Person * per) {
  infectious.erase(per);
}


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Population.cpp
//

#include "Population.hpp"
#include "Person.hpp"
#include <stdio.h>
#include "Random.hpp"
#include "Params.hpp"
#include "Profile.hpp"
#include <new>
#include "Strain.hpp"
#include "Global.hpp"


void Population::get_population_parameters() {
  get_param((char *) "popfile", popfile);
  get_param((char *) "profiles", profilefile);
  get_param((char *) "index_cases", &index_cases);
}


void Population::setup_population() {
  if (Verbose) {
    fprintf(Statusfp, "setup population entered\n");
    fflush(Statusfp);
  }

  int strains = Strain::get_strains();
  exposed = new (nothrow) set <int> [strains];
  if (exposed == NULL) { printf("Help! exposed allocation failure\n"); abort(); }

  infectious = new (nothrow) set <int> [strains];
  if (infectious == NULL) { printf("Help! infectious allocation failure\n"); abort(); }

  S = new (nothrow) int  [strains];
  if (S == NULL) { printf("Help! S allocation failure\n"); abort(); }

  E = new (nothrow) int  [strains];
  if (E == NULL) { printf("Help! E allocation failure\n"); abort(); }

  I = new (nothrow) int  [strains];
  if (I == NULL) { printf("Help! I allocation failure\n"); abort(); }

  R = new (nothrow) int  [strains];
  if (R == NULL) { printf("Help! R allocation failure\n"); abort(); }

  attack_rate = new (nothrow) double  [strains];
  if (attack_rate == NULL) { printf("Help! attack_rate allocation failure\n"); abort(); }

  // init population-strain lists
  for (int s = 0; s < strains; s++) {
    exposed[s].clear();
    infectious[s].clear();
    attack_rate[s] = 0.0;
    S[s] = E[s] = I[s] = R[s] = 0;
  }

  read_profiles(profilefile);
  read_population();

  if (Verbose) {
    fprintf(Statusfp, "setup population completed\n");
    fflush(Statusfp);
  }
}


void Population::read_population() {

  if (Verbose) {
    fprintf(Statusfp, "read population entered\n"); fflush(Statusfp);
  }

  // read in population
  FILE *fp = fopen(popfile, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "popfile %s not found\n", popfile);
    exit(1);
  }
  fscanf(fp, "Population = %d", &pop_size);
  if (Verbose) {
    fprintf(Statusfp, "Population = %d\n", pop_size);
    fflush(Statusfp);
  }

  // allocate population array
  pop = new (nothrow) Person [pop_size];
  if (pop == NULL) { printf ("Help! Pop allocation failure\n"); exit(1); }

  for (int p = 0; p < pop_size; p++) {
    int id, age, married, occ, prof, house, hood;
    int school, classroom, work, office, profile;
    char sex;

    // fprintf(Statusfp, "reading person %d\n", p); fflush(Statusfp); // DEBUG
    if (fscanf(fp, "%d %d %c %d %d %d %d %d %d %d %d %d %d",
	       &id, &age, &sex, &married, &occ, &prof, &house, &hood, &school,
	       &classroom, &work, &office, &profile) != 13) {
      fprintf(Statusfp, "Help! Read failure for person %d\n", p);
      abort();
    }
    pop[p].setup(id, age, sex, married, occ, prof,
		 Loc.get_location(house),
		 Loc.get_location(hood),
		 Loc.get_location(school),
		 Loc.get_location(classroom),
		 Loc.get_location(work),
		 Loc.get_location(office),
		 profile);
    pop[p].make_susceptible();
  }
  fclose(fp);
  if (Verbose) {
    fprintf(Statusfp, "finished reading population = %d\n", pop_size);
    fflush(Statusfp);
  }
}



void Population::population_quality_control() {

  if (Verbose) {
    fprintf(Statusfp, "population quality control check\n");
    fflush(Statusfp);
  }

  // check population
  for (int p = 0; p < pop_size; p++) {
    if (pop[p].get_places() == 0) {
      fprintf(Statusfp, "Help! Person %d has no home.\n",p);
      pop[p].print(0);
      continue;
    }
    if (pop[p].get_places() == 0) {
      fprintf(Statusfp,
	      "Help! Person %d has no place to go\n",p);
      pop[p].print(0);
    }
  }

  if (Verbose) {
    int count[20];
    int total = 0;
    // age distribution
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < pop_size; p++) {
      int a = pop[p].get_age();
      int n = a / 5;
      if (n < 20) { count[n]++; }
      else { count[19]++; }
      total++;
    }
    fprintf(Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     (c+1)*5, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    fprintf(Statusfp, "population quality control finished\n");
    fflush(Statusfp);
  }
}


void Population::start_outbreak() {
  // create index cases
  for (int i = 0; i < index_cases; i++) {
    int n = IRAND(0, pop_size-1);
    pop[n].make_exposed(0, -1, -1, 'X', 0);
  }
  E[0] = index_cases;
}

void Population::reset_population(int run) {

  if (Verbose) {
    fprintf(Statusfp, "reset population entered for run %d\n", run);
    fflush(Statusfp);
  }

  // init population-strain lists
  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    exposed[s].clear();
    infectious[s].clear();
    attack_rate[s] = 0.0;
    S[s] = E[s] = I[s] = R[s] = 0;
  }

  // add each person to the susceptible list for each place visited
  for (int p = 0; p < pop_size; p++){
    pop[p].make_susceptible();
  }

  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void Population::update_exposed_population(int day) {
  int p;
  set<int>::iterator it;
  stack <int> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_exposed for day %d\n", day);
    fflush(Statusfp);
  }

  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {

    for (it = exposed[s].begin(); it != exposed[s].end(); it++) {
      p = *it;
      if (pop[p].get_infectious_date(s) == day) {
	TempList.push(p);
      }
    }

    if (Verbose > 1) {
      fprintf(Statusfp,
	      "update_the_exposed: new infectious for day %d strain %d = %d\n",
	      day, s, (int) TempList.size());
      fflush(Statusfp);
    }

    while (!TempList.empty()) {
      p = TempList.top();
      pop[p].make_infectious(s);
      TempList.pop();
    }
  }

}

void Population::update_infectious_population(int day) {
  int p;
  set<int>::iterator it;
  stack <int> TempList;

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_infectious for day %d\n", day);
    fflush(Statusfp);
  }

  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {

    for (it = infectious[s].begin(); it != infectious[s].end(); it++) {
      p = *it;
      // printf("inf = %d recov = %d\n", p, pop[p].get_recovered_date(s));
      // fflush(stdout);
      if (pop[p].get_recovered_date(s) == day) {
	TempList.push(p);
      }
    }

    // printf("Templist size = %d\n", (int) TempList.size()); fflush(stdout);
    while (!TempList.empty()) {
      p = TempList.top();
      // printf("top = %d\n", p ); fflush(stdout);
      pop[p].make_recovered(s);
      TempList.pop();
    }
  }

  if (Verbose > 1) {
    fprintf(Statusfp, "update_the_infectious for day %d complete\n", day);
    fflush(Statusfp);
  }

}


void Population::update_population_behaviors(int day) {
  for (int p = 0; p < pop_size; p++) {
    pop[p].behave(day);
  }
}

void Population::update_population_stats(int day) {

  if (Verbose > 1) {
    fprintf(Statusfp, "update pop stats\n"); 
    fflush(Statusfp);
  }

  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    S[s] = E[s] = I[s] = R[s] = 0;
    for (int p = 0; p < pop_size; p++) {
      char status = pop[p].get_strain_status(s);
      S[s] += (status == 'S');
      E[s] += (status == 'E');
      I[s] += (status == 'I') || (status == 'i');
      R[s] += (status == 'R');
    }
    attack_rate[s] = (100.0*(E[s]+I[s]+R[s]))/pop_size;
  }
}

void Population::print_population_stats(int day) {

  if (Verbose > 1) {
    fprintf(Statusfp, "print pop stats for day %d\n", day);
    fflush(Statusfp);
  }

  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    int N = S[s]+E[s]+I[s]+R[s];
    fprintf(Outfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n",
	    day, s, S[s], E[s], I[s], R[s], N, attack_rate[s]);
    fflush(Outfp);

    if (Verbose) {
      fprintf(Statusfp,
	      "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n\n",
	      day, s, S[s], E[s], I[s], R[s], N, attack_rate[s]);
      fflush(Statusfp);
    }
  }
}

void Population::print_population() {
  for (int p = 0; p < pop_size; p++) {
    pop[p].print(0);
  }
}

void Population::insert_into_exposed_list(int strain, int per) {
  exposed[strain].insert(per);
}

void Population::insert_into_infectious_list(int strain, int per) {
  infectious[strain].insert(per);
}

void Population::remove_from_exposed_list(int strain, int per) {
  exposed[strain].erase(per);
}

void Population::remove_from_infectious_list(int strain, int per) {
  if (Verbose > 2) {
    printf("remove from infectious list person %d\n", per);
    printf("current size of infectious list = %d\n", (int) infectious[strain].size());
    fflush(stdout);
  }
  infectious[strain].erase(per);
  if (Verbose > 2) {
    printf("final size of infectious list = %d\n", (int) infectious[strain].size());
    fflush(stdout);
  }
}

int Population::get_age(int per) {
  return pop[per].get_age();
}

int Population::get_role(int per, int strain) {
  // return pop[per].get_role(strain);
  return 0;
}

char Population::get_strain_status(int per, int strain) {
  return pop[per].get_strain_status(strain);
}

int Population::is_place_on_schedule_for_person(int per, int day, int loc) {
  return pop[per].is_on_schedule(day, loc);
}

double Population::get_infectivity(int per, int strain) {
  return pop[per].get_infectivity(strain);
}

double Population::get_susceptibility(int per, int strain) {
  return pop[per].get_susceptibility(strain);
}

void Population::make_exposed(int per, int strain, int infector, int loc, char type, int day) {
  pop[per].make_exposed(strain, infector, loc, type, day);
}

void Population::add_infectee(int per, int strain) {
  pop[per].add_infectee(strain);
}

void Population::update_schedule(int per, int day) {
  pop[per].update_schedule(day);
}

void Population::get_schedule(int per, int *n, int *schedule) {
  pop[per].get_schedule(n, schedule);
}

double Population::get_attack_rate(int strain) {
  return attack_rate[strain];
}

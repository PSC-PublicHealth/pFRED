/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Population.cpp
//

#include <stdio.h>
#include <new>
#include "Population.hpp"
#include "Params.hpp"
#include "Profile.hpp"
#include "Global.hpp"
#include "Locations.hpp"
#include "Strain.hpp"

void Population::get_population_parameters() {
  get_param((char *) "popfile", popfile);
  get_param((char *) "profiles", profilefile);
  get_param((char *) "strains", &strains);
  extern int Strains;
  Strains = strains;
}

void Population::setup_population() {
  if (Verbose) {
    fprintf(Statusfp, "setup population entered\n");
    fflush(Statusfp);
  }
  strain = new Strain [strains];
  for (int s = 0; s < strains; s++) {
    strain[s].setup(s);
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
    pop[p].reset();
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
  for (int s = 0; s < strains; s++) {
    strain[s].start_outbreak(pop, pop_size);
  }
}


void Population::reset_population(int run) {

  if (Verbose) {
    fprintf(Statusfp, "reset population entered for run %d\n", run);
    fflush(Statusfp);
  }

  // reset population level infections
  for (int s = 0; s < strains; s++) {
    strain[s].reset();
  }

  // add each person to the susceptible list for each place visited
  for (int p = 0; p < pop_size; p++){
    pop[p].reset();
  }

  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void Population::update(int day) {
  update_exposed_population(day);
  update_infectious_population(day);
  update_population_behaviors(day);
  Loc.process_infectious_locations(day);
}

void Population::report(int day) {
  update_population_stats(day);
  print_population_stats(day);
}


void Population::update_exposed_population(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].update_exposed(day);
  }
}

void Population::update_infectious_population(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].update_infectious(day);
  }
}


void Population::update_population_behaviors(int day) {
  for (int p = 0; p < pop_size; p++) {
    pop[p].behave(day);
  }
}

void Population::update_population_stats(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].update_stats(pop, pop_size, day);
  }
}

void Population::print_population_stats(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].print_stats(day);
  }
}

void Population::print_population() {
  for (int p = 0; p < pop_size; p++) {
    pop[p].print(0);
  }
}

Strain * Population::get_strain(int s) {
  return &strain[s];
}


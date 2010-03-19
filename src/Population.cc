/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Population.cc
//

#include <stdio.h>
#include <new>
#include <string>
#include <sstream>
#include "Population.h"
#include "Params.h"
#include "Profile.h"
#include "Global.h"
#include "Locations.h"
#include "Strain.h"
#include "Person.h"
#include "AgeMap.h"
#include "Random.h"

using namespace std; 

void Population::get_parameters() {
  get_param((char *) "popfile", popfile);
  get_param((char *) "profiles", profilefile);
  get_param((char *) "strains", &strains);
  extern int Strains;
  Strains = strains;

  int num_mutation_params =
    get_param_matrix((char *) "mutation_prob", &mutation_prob);
  if (num_mutation_params != strains) {
    fprintf(Statusfp,
	    "Improper mutation matrix: expected square matrix of %i rows, found %i",
	    strains, num_mutation_params);
    exit(1);
  }
  if (Verbose > 1) {
    printf("\nmutation_prob:\n");
    for (int i  = 0; i < strains; i++)  {
      for (int j  = 0; j < strains; j++) {
	printf("%f ", mutation_prob[i][j]);
      }
      printf("\n");
    }
  }

}

void Population::setup() {
  if (Verbose) {
    fprintf(Statusfp, "setup population entered\n");
    fflush(Statusfp);
  }
  strain = new Strain [strains];
  for (int is = 0; is < strains; is++) {
    strain[is].setup(is, this, mutation_prob[is]);
  }
  read_profiles(profilefile);
  read_population();
  if (Verbose) {
    fprintf(Statusfp, "setup population completed, strains = %d\n", strains);
    fflush(Statusfp);
  }
  AVs.setup();
  char s[80];
  
  for(int is = 0; is< strains; is++){
    sprintf(s,"residual_immunity_ages[%d]",is);
    if(does_param_exist(s)){
      residual_immunity = new AgeMap("Residual Immunity");
      stringstream ss; 
      ss << "residual_immunity[" << is<< "]";
      residual_immunity->read_from_input(ss.str());
      residual_immunity->print();
      
    }
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

void Population::reset(int run) {
  if (Verbose) {
    fprintf(Statusfp, "reset population entered for run %d\n", run);
    fflush(Statusfp);
  }
  // reset population level infections
  for (int s = 0; s < strains; s++) {
    strain[s].reset();
  }

  // reset each person (adds her to the susc list for each favorite place)
  for (int p = 0; p < pop_size; p++){
    pop[p].reset();
  }
  
  if(residual_immunity != NULL){
    for(int is =0;is < strains; is++){
      apply_residual_immunity(strain[is]);
    }
  }
  AVs.reset();

  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void Population::update(int day) {
  //if(AV.number_antivirals > 0)
  //  AV.update(strains,pop,pop_size,day); // This allows me to put a set of medication for each population
  // This will be the place the "manager" can allocate AVs
  int imm = 0;
  for (int p = 0; p < pop_size; p++){
    if(pop[p].get_strain_status(0)=='M'){
      imm++;
    }
    pop[p].update(day);
  }
  if(Debug == 1){
    cout << "residual immunity on day " << day << " is " << imm << "\n";
  }
  
  for (int s = 0; s < strains; s++) {
    strain[s].update(day);
  }
}

void Population::report(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].update_stats(day);
    strain[s].print_stats(day);
  }
}

void Population::print() {
  for (int p = 0; p < pop_size; p++) {
    pop[p].print(0);
  }
}

void Population::end_of_run() {
  for (int p = 0; p < pop_size; p++) {
    for (int s = 0; s < strains; s++) {
      if (pop[p].get_strain_status(s) != 'R')
	pop[p].print(s);
    }
  }
}

Strain * Population::get_strain(int s) {
  return &strain[s];
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
      int n = a / 10;
      if (n < 20) { count[n]++; }
      else { count[19]++; }
      total++;
    }
    fprintf(Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 10; c++) {
      fprintf(Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
	      10*c, 10*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    fprintf(Statusfp, "population quality control finished\n");
    fflush(Statusfp);
  }
}

void Population::apply_residual_immunity(Strain strain){
  int numimmune = 0;
  for( int ip = 0; ip < pop_size; ip++){
    Person* p = &pop[ip];
    int age = p->get_age();
    int value = residual_immunity->find_value(age);
    //cout << "\n Age = "<<age << " Value = "<<value;
    if(RANDOM()*100. < value){
      p->get_health()->immunize(&strain);
      numimmune++;
    }
  }
  
  cout << "\nNumber of People made immune from residual immunity " << numimmune;
}

      

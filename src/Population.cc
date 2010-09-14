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
#include "Demographics.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "Vaccine_Manager.h"
#include "Age_Map.h"
#include "Random.h"

using namespace std; 

// global singleton object
Population Pop;

int V_count;

Population::Population() {
  pop = NULL;
  pop_size = -1;
  strain = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  strains = -1;
  mutation_prob = NULL;
  pregnancy_prob = NULL;
}

Population::~Population() {
  if(pop != NULL) delete [] pop;
  if(strain != NULL) delete [] strain;
  if(vacc_manager != NULL) delete vacc_manager;
  if(av_manager != NULL) delete av_manager;
  if(mutation_prob != NULL) delete [] mutation_prob;
  if(pregnancy_prob != NULL) delete pregnancy_prob;
}


void Population::get_parameters() {
  get_param((char *) "popfile", popfile);
  get_param((char *) "profiles", profilefile);
  strains = Strains;
  
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
  
  // Setup the pregnancy probability map
  pregnancy_prob = new Age_Map("Pregnancy Probability");
  pregnancy_prob->read_from_input("pregnancy_prob");
  pregnancy_prob->print();	
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
  
  vacc_manager = new Vaccine_Manager(this);
  av_manager   = new AV_Manager(this);
  if(Verbose > 1) av_manager->print();
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
  if (1!=fscanf(fp, "Population = %d", &pop_size)){
    fprintf(Statusfp, "failed to parse pop_size\n");
    exit(1);
  }
  if (Verbose) {
    fprintf(Statusfp, "Population = %d\n", pop_size);
    fflush(Statusfp);
  }
  
  // allocate population array
  pop = new (nothrow) Person* [pop_size];
	
  // clear age-related lists
  children.clear();
  adults.clear();

  for (int i = 0; i < pop_size; i++)
    pop[i] = new Person;
	
  if (pop == NULL) { printf ("Help! Pop allocation failure\n"); exit(1); }
  
  for (int p = 0; p < pop_size; p++) {
    int id, age, married, prof, house, hood;
    int school, classroom, work, office, profile;
    char sex;
    
    // fprintf(Statusfp, "reading person %d\n", p); fflush(Statusfp); // DEBUG
    if (fscanf(fp, "%d %d %c %d %d %d %d %d %d %d %d %d %*s",
               &id, &age, &sex, &married, &prof, &house, &hood, &school,
               &classroom, &work, &office, &profile) != 12) {
      fprintf(Statusfp, "Help! Read failure for person %d\n", p);
      abort();
    }
    
    Place *favorite_place[] = { 
      Loc.get_location(house),
      Loc.get_location(hood),
      Loc.get_location(school),
      Loc.get_location(classroom),
      Loc.get_location(work),
      Loc.get_location(office)
    }; 
    pop[p]->setup(id, age, sex, married, prof, favorite_place, profile, this);
    pop[p]->reset();
    if (age < 18) {
      children.push_back(pop[p]);
    }
    else {
      adults.push_back(pop[p]);
    }
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
  
  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  
  // reset each person (adds her to the susc list for each favorite place)
  for (int p = 0; p < pop_size; p++){
    pop[p]->reset();
    never_changed[pop[p]]=true; // add all agents to this list at start
  }
  if(Verbose > 0){
    int count = 0;
    for(int p = 0; p < pop_size; p++){
      Strain* s = &strain[0];
      if(pop[p]->get_health()->is_immune(s)) count++;
    }
    cout << "Number of Residually Immune people = "<<count << "\n";
  }
  av_manager->reset();
  vacc_manager->reset();
  
  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void Population::update(int day) {

  // update adults first, so that they can make decisions for minors
  vector<Person *>::iterator itr;
  for (itr = adults.begin(); itr != adults.end(); itr++) {
    (*itr)->update(day);
  }

  // update minors, who may use adult's decisions
  for (itr = children.begin(); itr != children.end(); itr++) {
    (*itr)->update(day);
  }

  vacc_manager->update(day);
  av_manager->update(day);
  for (int s = 0; s < strains; s++) {
    strain[s].update(day);
  }
  av_manager->disseminate(day);
}

void Population::report(int day) {
  for (int s = 0; s < strains; s++) {
    strain[s].update_stats(day);
    strain[s].print_stats(day);
  }
}

void Population::print(int incremental, int day) {
  if (Tracefp == NULL) return;

  if (!incremental){
    if (Trace_Headers) fprintf(Tracefp, "# All agents, by ID\n");
    for (int p = 0; p < pop_size; p++)
      for (int i=0; i<strains; i++)
	pop[p]->print(i);
  } else if (1==incremental) {
    ChangeMap::const_iterator iter;
		
    if (Trace_Headers){
      if (day<Days)
	fprintf(Tracefp, "# Incremental Changes (every %d): Day %3d\n", Incremental_Trace, day);
      else
	fprintf(Tracefp, "# End-of-simulation: Remaining unreported changes\n");
			
      if (! incremental_changes.size()){
	fprintf(Tracefp, "# <LIST-EMPTY>\n");
	return;
      }
    }
		
    for (iter = this->incremental_changes.begin();
         iter != this->incremental_changes.end();
         iter++){
      (iter->first)->print(0); // the map key is a Person*
    }
  } else {
    ChangeMap::const_iterator iter;
    if (Trace_Headers){
      fprintf(Tracefp, "# Agents that never changed\n");
      if (! never_changed.size()){
	fprintf(Tracefp, "# <LIST-EMPTY>\n");
	return;
      }
    }
		
    for (iter = this->never_changed.begin();
         iter != this->never_changed.end();
         iter++){
      (iter->first)->print(0); // the map key is a Person*
    }
  }
	
  // empty out the incremental list of Person's who have changed
  if (-1<incremental)
    incremental_changes.clear();
}

void Population::end_of_run() {
  // print out agents who have changes yet unreported
  // (results from Incremental_Trace==0  || Days%Incremental_Trace!=0)
  this->print(1, Days);
  
  // print out all those agents who never changed
  this->print(-1);
}

Strain *Population::get_strain(int s) {
  return &strain[s];
}

void Population::population_quality_control() {
  if (Verbose) {
    fprintf(Statusfp, "population quality control check\n");
    fflush(Statusfp);
  }
  
  // check population
  for (int p = 0; p < pop_size; p++) {
    if (pop[p]->get_behavior()->get_household() == NULL) {
      fprintf(Statusfp, "Help! Person %d has no home.\n",p);
      pop[p]->print(0);
    }
  }
  
  if (Verbose) {
    int count[20];
    int total = 0;
    // age distribution
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < pop_size; p++) {
      int a = pop[p]->get_age();
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
    
    if (pregnancy_prob->get_num_ages() > 0){
      int pcount[20];
      // Print out pregnancy distribution
      for (int c = 0; c < 20; c++) { pcount[c] = 0; }
      for (int p = 0; p < pop_size; p++) {
        int a = pop[p]->get_age();
        int n = a / 10;
        if(pop[p]->get_demographics()->is_pregnant()==true) {
          if( n < 20 ) { pcount[n]++; }
          else { pcount[19]++; }
        }
      } 
      fprintf(Statusfp, "\nAge Distribution of Pregnancy: %d people\n", total);
      for(int c = 0; c < 10; c++) {
        fprintf(Statusfp, "age %2d to %2d: %6d (%.2f%%)\n",
                10*c, 10*(c+1)-1, pcount[c], (100.0*pcount[c])/total);
      }
      fprintf(Statusfp,"\n");
    }
    
    // Print out At Risk distribution
    for(int is = 0; is < strains; is++){
      if(strain[is].get_at_risk()->get_num_ages() > 0){
        Strain* s = &strain[is];
        int rcount[20];
        for (int c = 0; c < 20; c++) { rcount[c] = 0; }
        for (int p = 0; p < pop_size; p++) {
          int a = pop[p]->get_age();
          int n = a / 10;
          if(pop[p]->get_health()->is_at_risk(s)==true) {
            if( n < 20 ) { rcount[n]++; }
            else { rcount[19]++; }
          }
        }
        fprintf(Statusfp, "\n Age Distribution of At Risk for Strain %d: %d people\n",is,total);
        for(int c = 0; c < 10; c++ ) {
          fprintf(Statusfp, "age %2d to %2d: %6d (%.2f%%)\n",
                  10*c, 10*(c+1)-1, rcount[c], (100.0*rcount[c])/total);
        }
        fprintf(Statusfp, "\n");
      }
    }	
  }
  
  
  if (Verbose) {
    fprintf(Statusfp, "population quality control finished\n");
    fflush(Statusfp);
  }
}

void Population::set_changed(Person *p){
  incremental_changes[p]=true; // note that this element has been changed
  never_changed.erase(p);      // remove it from this list 
  // (might already be gone, but this won't hurt)
}

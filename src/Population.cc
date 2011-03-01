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
#include "Place_List.h"
#include "Disease.h"
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
  disease = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  diseases = -1;
  mutation_prob = NULL;
  pregnancy_prob = NULL;
}

Population::~Population() {
  if(pop != NULL) delete [] pop;
  if(disease != NULL) delete [] disease;
  if(vacc_manager != NULL) delete vacc_manager;
  if(av_manager != NULL) delete av_manager;
  if(mutation_prob != NULL) delete [] mutation_prob;
  if(pregnancy_prob != NULL) delete pregnancy_prob;
}


void Population::get_parameters() {
  get_param((char *) "popfile", popfile);
  get_param((char *) "profiles", profilefile);
  diseases = Diseases;
  
  int num_mutation_params =
    get_param_matrix((char *) "mutation_prob", &mutation_prob);
  if (num_mutation_params != diseases) {
    fprintf(Statusfp,
            "Improper mutation matrix: expected square matrix of %i rows, found %i",
            diseases, num_mutation_params);
    exit(1);
  }
  if (Verbose > 1) {
    printf("\nmutation_prob:\n");
    for (int i  = 0; i < diseases; i++)  {
      for (int j  = 0; j < diseases; j++) {
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
  disease = new Disease [diseases];
  for (int is = 0; is < diseases; is++) {
    disease[is].setup(is, this, mutation_prob[is]);
  }
  read_profiles(profilefile);
  read_population();
  if (Verbose) {
    fprintf(Statusfp, "setup population completed, diseases = %d\n", diseases);
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
  char population_file[256];
  sprintf(population_file, "%s/%s", Population_directory, popfile);
  FILE *fp = fopen(population_file, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "popultion_file %s not found\n", population_file);
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
    
    // ignore the neighborhood entry
    hood = -1;

    Place *favorite_place[] = { 
      Places.get_place(house),
      Places.get_place(hood),
      Places.get_place(school),
      Places.get_place(classroom),
      Places.get_place(work),
      Places.get_place(office)
    }; 
    pop[p]->setup(id, age, sex, married, prof, favorite_place, profile, this);
    pop[p]->reset();
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
  for (int s = 0; s < diseases; s++) {
    disease[s].reset();
  }
  
  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  
  for (int p = 0; p < pop_size; p++){
    pop[p]->reset();
    never_changed[pop[p]]=true; // add all agents to this list at start
  }
  if(Verbose > 0){
    int count = 0;
    for(int p = 0; p < pop_size; p++){
      Disease* s = &disease[0];
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

  // update everyone's demographics
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_demographics(day);
  }

  // update everyone's health status
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_health(day);
  }

  // update adult decisions
  for (int p = 0; p < pop_size; p++) {
    if (18 <= pop[p]->get_age()) {
      pop[p]->update_cognition(day);
    }
  }

  // update child decisions
  for (int p = 0; p < pop_size; p++) {
    if (pop[p]->get_age() < 18) {
      pop[p]->update_cognition(day);
    }
  }

  // update actions of infectious agents
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_infectious_behavior(day);
  }

  // update actions of susceptible agents
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_susceptible_behavior(day);
  }

  // distribute vaccines
  vacc_manager->update(day);

  // distribute AVs
  av_manager->update(day);

  // apply transmission model in all infectious places
  for (int s = 0; s < diseases; s++) {
    disease[s].update(day);
  }

  // give out anti-virals (after today's infections)
  av_manager->disseminate(day);
}


void Population::report(int day) {
  for (int s = 0; s < diseases; s++) {
    disease[s].update_stats(day);
    disease[s].print_stats(day);
  }
}

void Population::print(int incremental, int day) {
  if (Tracefp == NULL) return;

  if (!incremental){
    if (Trace_Headers) fprintf(Tracefp, "# All agents, by ID\n");
    for (int p = 0; p < pop_size; p++)
      for (int i=0; i<diseases; i++)
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

Disease *Population::get_disease(int s) {
  return &disease[s];
}

void Population::quality_control() {
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
    for(int is = 0; is < diseases; is++){
      if(disease[is].get_at_risk()->get_num_ages() > 0){
        Disease* s = &disease[is];
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
        fprintf(Statusfp, "\n Age Distribution of At Risk for Disease %d: %d people\n",is,total);
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

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

//Static variables
int Population::next_id = 0;

Population::Population() {

  pop.clear();
  pop_map.clear();
  pop_size = 0;
  disease = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  diseases = -1;
  mutation_prob = NULL;
  population_changed = false;
}

Population::~Population() {

  // free all memory (remember, first delete the referenced memory before
  // deleting the pointer to it --as the vector is an array of pointers.
  for (unsigned i = 0; i < pop.size(); ++i)
    delete pop[i];
  pop.clear();
  pop_map.clear();

  pop_size = 0;
  if(disease != NULL) delete [] disease;
  if(vacc_manager != NULL) delete vacc_manager;
  if(av_manager != NULL) delete av_manager;
  if(mutation_prob != NULL) delete [] mutation_prob;
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
  
}

void Population::add_person(Person * person) {
  assert(pop_map.find(person) == pop_map.end());
  pop.push_back(person);
  assert(pop_size == pop.size()-1);
  pop_map[person] = pop_size;
  pop_size = pop.size();
  population_changed = true;

}

void Population::delete_person(Person * person) {
  map<Person *,int>::iterator it;
  // printf("POP_MAP DELETE: %d\n", person->get_id());
  it = pop_map.find(person);
  if (it == pop_map.end()) {
    printf("Help! person %d deleted, but not in the pop_map\n", person->get_id());
  }
  assert(it != pop_map.end());
  int n = (*it).second;
  Person * last = pop[pop_size-1];
  pop[n] = last;
  pop.pop_back();
  pop_size--;
  pop_map.erase(it);
  pop_map[last] = n;
  if (pop_size != pop.size()) { printf("pop_size = %d  pop.size() = %d\n",
				       pop_size, (int) pop.size()); }
  assert(pop_size == pop.size());
  graveyard.push_back(person);
  population_changed = true;
}

void Population::prepare_to_die(Person *per) {
  //Add person to daily death_list
  death_list.push_back(per);
}

void Population::prepare_for_birth(Person *per) {
  //Add person to daily birth_list
  birth_list.push_back(per);
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
    fprintf(Statusfp, "population_file %s not found\n", population_file);
    exit(1);
  }
  int psize;
  if (1!=fscanf(fp, "Population = %d", &psize)){
    fprintf(Statusfp, "failed to parse pop_size\n");
    exit(1);
  }
  if (Verbose) {
    fprintf(Statusfp, "Population = %d\n", psize);
    fflush(Statusfp);
  }
  
  // reserve population vector
  pop.reserve(psize);
	
  for (int p = 0; p < psize; p++) {
    Person * person = new Person;
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
    
    //Increment the static next_id and make certain that it is always at least one greater than the max id read in
    if(Population::next_id <= id)
      Population::next_id = id + 1;

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

    person->setup(id, age, sex, married, prof, favorite_place, profile, this, Sim_Start_Date);
    person->reset(Sim_Start_Date);
    person->register_event_handler(this);
    add_person(person);
  }

  fclose(fp);
  assert(pop_size == psize);
  population_changed = false;
  
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

  if (population_changed) {
    pop.clear();
    pop_map.clear();
    pop_size = 0;
    read_population();
  }

  // reset population level infections
  for (int s = 0; s < diseases; s++) {
    disease[s].reset();
  }
  
  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  
  for (int p = 0; p < pop_size; p++){
    pop[p]->reset(Sim_Start_Date);
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
  population_changed = false;
  
  if (Verbose) {
    fprintf(Statusfp, "reset population completed\n");
    fflush(Statusfp);
  }
}

void Population::update(int day) {

  death_list.clear();
  birth_list.clear();

  // update everyone's demographics
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_demographics(Sim_Start_Date, day);
  }

  // update everyone's health status
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_health(day);
  }

  // add the births to the population
  size_t births = birth_list.size();

  for (size_t i = 0; i < births; i++) {
    Person * baby = birth_list[i]->give_birth();
    baby->register_event_handler(this);
    add_person(baby);
  }

  if (Verbose) {
    fprintf(Statusfp, "births = %d\n", (int)births);fflush(stdout);
  }

  // remove the dead from the population
  size_t deaths = death_list.size();
  for (size_t i = 0; i < deaths; i++) {
    delete_person(death_list[i]);
  }

  if (Verbose) {
    fprintf(Statusfp, "deaths = %d\n", (int)deaths);fflush(stdout);
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
    pop[p]->update_infectious_behavior(Sim_Start_Date, day);
  }

  // update actions of susceptible agents
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_susceptible_behavior(Sim_Start_Date, day);
  }

  // distribute vaccines
  vacc_manager->update(day);

  // distribute AVs
  av_manager->update(day);

  // apply transmission model in all infectious places
  for (int s = 0; s < diseases; s++) {
    disease[s].update(Sim_Start_Date, day);
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
  this->print(1, Days);
  
  // print out all those agents who never changed
  this->print(-1);

  // free all memory (remember, first delete the referenced memory before
  // deleting the pointer to it --as the vector is an array of pointers.
  for (unsigned i = 0; i < pop.size(); ++i)
    delete pop[i];

  pop.clear();
  pop_map.clear();
  birth_list.clear();

  this->pop_size = 0;

  // loop over dead agents
  for (unsigned i = 0; i < graveyard.size(); ++i)
    delete graveyard[i];

  graveyard.clear();
  death_list.clear();

  this->read_population();

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

//Implement the interface
void Population::handle_property_change_event(Person *source, string property_name, int prev_val, int new_val) {

}

void Population::handle_property_change_event(Person *source, string property_name, bool new_val) {

  if (property_name.compare("deceased") == 0 && new_val) {
    this->prepare_to_die(source);
  } else if (property_name.compare("deliver") == 0 && new_val) {
    this->prepare_for_birth(source);
  }
}


//Static function to get and increment the next_id
int Population::get_next_id() {
  int return_val = Population::next_id;
  Population::next_id++;
  return return_val;
}


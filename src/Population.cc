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
#include "Date.h"

using namespace std; 

// global singleton object
Population Pop;

// used for reporting
int V_count;
int age_count_male[Demographics::MAX_AGE + 1];
int age_count_female[Demographics::MAX_AGE + 1];
int birth_count[Demographics::MAX_AGE + 1];
int death_count_male[Demographics::MAX_AGE + 1];
int death_count_female[Demographics::MAX_AGE + 1];

Population::Population() {
  clear_static_arrays();
  pop.clear();
  pop_map.clear();
  pop_size = 0;
  disease = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  diseases = -1;
  mutation_prob = NULL;
  next_id = 0;
}

Population::~Population() {
  // free all memory
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
  assert((unsigned) pop_size == pop.size()-1);
  pop_map[person] = pop_size;
  pop_size = pop.size();
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
  if ((unsigned) pop_size != pop.size()) {
    printf("pop_size = %d  pop.size() = %d\n",
	   pop_size, (int) pop.size());
  }
  assert((unsigned) pop_size == pop.size());
  graveyard.push_back(person);
}

void Population::prepare_to_die(int day, Person *per) {
  // add person to daily death_list
  death_list.push_back(per);
  report_death(day, per);
  if (Verbose>1) {
    fprintf(Statusfp, "prepare to die: ");
    per->print(Statusfp,0);
  }
}

void Population::prepare_to_give_birth(int day, Person *per) {
  // add person to daily maternity_list
  maternity_list.push_back(per);
  report_birth(day, per);
  if (Verbose>1) {
    fprintf(Statusfp,"prepare to give birth: ");
    per->print(Statusfp,0);
  }
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
  vacc_manager = new Vaccine_Manager(this);
  av_manager   = new AV_Manager(this);
  if (Verbose > 1) av_manager->print();
  pop.clear();
  pop_map.clear();
  pop_size = 0;
  maternity_list.clear();
  graveyard.clear();
  death_list.clear();
  read_population();

  // clear population level infections
  for (int s = 0; s < diseases; s++) {
    disease[s].clear();
  }
  
  // clear individuals
  for (int p = 0; p < pop_size; p++){
    pop[p]->reset(Sim_Date);
  }

  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  for (int p = 0; p < pop_size; p++){
    never_changed[pop[p]]=true; // add all agents to this list at start
  }

  if(Verbose > 0){
    int count = 0;
    for(int p = 0; p < pop_size; p++){
      Disease* s = &disease[0];
      if(pop[p]->get_health()->is_immune(s)) count++;
    }
    fprintf(Statusfp, "number of residually immune people = %d\n", count);
    fflush(Statusfp);
  }
  av_manager->reset();
  vacc_manager->reset();
  
  if (Verbose) {
    fprintf(Statusfp, "population setup finished\n");
    fflush(Statusfp);
  }
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
	
  // skip header line
  fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s");

  next_id = 0;
  for (int p = 0; p < psize; p++) {
    Person * person = new Person;
    int age, married, occ;
    char label[32], house[32], school[32], work[32];
    char sex;
    // fprintf(Statusfp, "reading person %d\n", p); fflush(Statusfp); // DEBUG
    if (fscanf(fp, "%s %d %c %d %d %s %s %s",
               label, &age, &sex, &married, &occ, house, school, work) != 8) {
      fprintf(Statusfp, "Help! Read failure for new person %d\n", p);
      abort();
    }
    // printf("new person: %d %s %d %c %d %d %s %s %s\n",
    // next_id, label, age, sex, married, occ, house, school, work);
    // fflush(stdout);
    person->setup(next_id, age, sex, married, occ, house, school, work, this, Sim_Date, true);
    person->register_event_handler(this);
    add_person(person);
    next_id++;
  }
  fclose(fp);
  assert(pop_size == psize);
  if (Verbose) {
    fprintf(Statusfp, "finished reading population = %d\n", pop_size);
    fflush(Statusfp);
  }
}

void Population::begin_day(int day) {

  // clear lists of births and deaths
  death_list.clear();
  maternity_list.clear();

  // update activity profiles on July 1
  if (Enable_Aging && strcmp(Sim_Date->get_MMDD(day), "07-01")==0) {
    for (int p = 0; p < pop_size; p++) {
      pop[p]->update_activity_profile();
    }
  }

  // update everyone's demographics
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_demographics(Sim_Date, day);
  }

  // update everyone's health status
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_health(day);
  }

  // add the births to the population
  size_t births = maternity_list.size();
  for (size_t i = 0; i < births; i++) {
    Person * baby = maternity_list[i]->give_birth(day);
    baby->register_event_handler(this);
    add_person(baby);
    int age_lookup = maternity_list[i]->get_age();
    if (age_lookup > Demographics::MAX_AGE)
      age_lookup = Demographics::MAX_AGE;
    birth_count[age_lookup]++;
  }
  if (Verbose) {
    fprintf(Statusfp, "births = %d\n", (int)births);fflush(stdout);
  }

  // remove the dead from the population
  size_t deaths = death_list.size();
  for (size_t i = 0; i < deaths; i++) {
    //For reporting
    int age_lookup = death_list[i]->get_age();
    if (age_lookup > Demographics::MAX_AGE)
      age_lookup = Demographics::MAX_AGE;
    if (death_list[i]->get_sex() == 'F')
      death_count_female[age_lookup]++;
    else
      death_count_male[age_lookup]++;
    delete_person(death_list[i]);
  }
  if (Verbose) {
    fprintf(Statusfp, "deaths = %d\n", (int)deaths);fflush(stdout);
  }

  // update adult decisions
  for (int p = 0; p < pop_size; p++) {
    if (ADULT_AGE <= pop[p]->get_age()) {
      pop[p]->update_behavior(day);
    }
  }

  // update child decisions
  for (int p = 0; p < pop_size; p++) {
    if (pop[p]->get_age() < ADULT_AGE) {
      pop[p]->update_behavior(day);
    }
  }

  // distribute vaccines
  vacc_manager->update(day);

  // distribute AVs
  av_manager->update(day);

  if (Verbose>1) {
    fprintf(Statusfp, "population begin_day finished\n");fflush(stdout);
  }
}

void Population::get_visitors_to_infectious_places(int day) {

  // find places visited by infectious agents
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_infectious_activities(Sim_Date, day);
  }

  // add susceptibles to infectious places
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_susceptible_activities(Sim_Date, day);
  }

  if (Verbose>1) {
    fprintf(Statusfp, "find_infectious_places finished\n");fflush(stdout);
  }

}

void Population::transmit_infection(int day) {

  // apply transmission model in all infectious places
  for (int s = 0; s < diseases; s++) {
    if (Verbose > 1) {
      fprintf(Statusfp, "disease = %d day = %d\n",s,day);fflush(stdout);
      disease[s].print();
    }
    disease[s].update(Sim_Date, day);
  }

  if (Verbose>1) {
    fprintf(Statusfp, "find_infectious_places finished\n");fflush(stdout);
  }

}

void Population::end_day(int day) {

  // give out anti-virals (after today's infections)
  av_manager->disseminate(day);

  if (Verbose && strcmp(Sim_Date->get_MMDD(day), "12-31")==0) {
    // print the statistics on December 31 of each year
    for (int i = 0; i < pop_size; ++i) {
      int age_lookup = pop[i]->get_age();
      if (age_lookup > Demographics::MAX_AGE)
	age_lookup = Demographics::MAX_AGE;
      if (pop[i]->get_sex() == 'F')
	age_count_female[age_lookup]++;
      else
	age_count_male[age_lookup]++;
    }
    for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
      int count, num_deaths, num_births;
      double birthrate, deathrate;
      fprintf(Statusfp,
	      "DEMOGRAPHICS Year %d TotalPop %d Age %d ", 
	      Sim_Date->get_year(day), pop_size, i);
      count = age_count_female[i];
      num_births = birth_count[i];
      num_deaths = death_count_female[i];
      birthrate = count>0 ? ((100.0*num_births)/count) : 0.0;
      deathrate = count>0 ? ((100.0*num_deaths)/count) : 0.0;
      fprintf(Statusfp,
	      "count_f %d births_f %d birthrate_f %.03f deaths_f %d deathrate_f %.03f ",
	      count, num_births, birthrate, num_deaths, deathrate);
      count = age_count_male[i];
      num_deaths = death_count_male[i];
      deathrate = count?((100.0*num_deaths)/count):0.0;
      fprintf(Statusfp,
	      "count_m %d deaths_m %d deathrate_m %.03f\n",
	      count, num_deaths, deathrate);
      fflush(stdout);
    }
    clear_static_arrays();
  }

  if (Verbose > 1) {
    fprintf(Statusfp, "population update finished\n");fflush(stdout);
  }

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
	pop[p]->print(Tracefp, i);
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
      (iter->first)->print(Tracefp, 0); // the map key is a Person*
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
      (iter->first)->print(Tracefp, 0); // the map key is a Person*
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
    if (pop[p]->get_activities()->get_household() == NULL) {
      fprintf(Statusfp, "Help! Person %d has no home.\n",p);
      pop[p]->print(Statusfp, 0);
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

//Static function to clear arrays
void Population::clear_static_arrays() {
  for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
    age_count_male[i] = 0;
    age_count_female[i] = 0;
    death_count_male[i] = 0;
    death_count_female[i] = 0;
  }
  for (int i = 0; i <= Demographics::MAX_AGE; ++i) {
    birth_count[i] = 0;
  }
}

//Implement the interface
void Population::handle_property_change_event(Person *source,
					      string property_name,
					      int prev_val,
					      int new_val) {
}

void Population::handle_property_change_event(Person *source,
					      string property_name,
					      bool new_val) {
  /*
  if (property_name.compare("deceased") == 0 && new_val) {
    this->prepare_to_die(source);
  } else if (property_name.compare("deliver") == 0 && new_val) {
    this->prepare_to_give_birth(source);
  }
  */
}


//Static function to get and increment the next_id
int Population::get_next_id() {
  int return_val = next_id;
  next_id++;
  return return_val;
}

void Population::assign_classrooms() {
  if (Verbose) {
    fprintf(Statusfp, "assign classrooms entered\n"); fflush(Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    pop[p]->assign_classroom();
  }
  if (Verbose) {
    fprintf(Statusfp, "assign classrooms finished\n"); fflush(Statusfp);
  }
}

void Population::assign_offices() {
  if (Verbose) {
    fprintf(Statusfp, "assign offices entered\n"); fflush(Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    pop[p]->assign_office();
  }
  if (Verbose) {
    fprintf(Statusfp, "assign offices finished\n"); fflush(Statusfp);
  }
}

void Population::report_birth(int day, Person *per) const {
  if (Birthfp == NULL) return;
  fprintf(Birthfp, "day %d mother %d age %d\n",
	  day,
	  per->get_id(),
	  per->get_age());
  fflush(Birthfp);
}

void Population::report_death(int day, Person *per) const {
  if (Deathfp == NULL) return;
  fprintf(Deathfp, "day %d person %d age %d\n",
	  day,
	  per->get_id(),
	  per->get_age());
  fflush(Deathfp);
}


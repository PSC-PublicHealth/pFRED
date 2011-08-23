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
#include "Travel.h"
#include "Utils.h"

using namespace std; 

char ** pstring;

// used for reporting
int V_count;
int age_count_male[Demographics::MAX_AGE + 1];
int age_count_female[Demographics::MAX_AGE + 1];
int birth_count[Demographics::MAX_AGE + 1];
int death_count_male[Demographics::MAX_AGE + 1];
int death_count_female[Demographics::MAX_AGE + 1];

char Population::popfile[256];
char Population::profilefile[256];
char Population::pop_outfile[256];
char Population::output_population_date_match[256];
int Population::output_population = 0;
bool Population::is_initialized = false;
int Population::next_id = 0;

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
  get_param((char *) "popfile", Population::popfile);
  get_param((char *) "profiles", Population::profilefile);
  diseases = Global::Diseases;
  
  int num_mutation_params =
    get_param_matrix((char *) "mutation_prob", &mutation_prob);
  if (num_mutation_params != diseases) {
    fprintf(Global::Statusfp,
            "Improper mutation matrix: expected square matrix of %i rows, found %i",
            diseases, num_mutation_params);
    exit(1);
  }

  if (Global::Verbose > 1) {
    printf("\nmutation_prob:\n");
    for (int i  = 0; i < diseases; i++)  {
      for (int j  = 0; j < diseases; j++) {
        printf("%f ", mutation_prob[i][j]);
      }
      printf("\n");
    }
  }

  //Only do this one time
  if(!Population::is_initialized) {
    get_param((char *) "output_population", &Population::output_population);
    if(Population::output_population > 0) {
      get_param((char *) "pop_outfile", Population::pop_outfile);
      get_param((char *) "output_population_date_match", Population::output_population_date_match);
    }
    Population::is_initialized = true;
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
  person->withdraw_from_activities();
  delete person;
  // graveyard.push_back(person);
}

void Population::prepare_to_die(int day, Person *per) {
  // add person to daily death_list
  death_list.push_back(per);
  report_death(day, per);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "prepare to die: ");
    per->print(Global::Statusfp,0);
  }
}

void Population::prepare_to_give_birth(int day, Person *per) {
  // add person to daily maternity_list
  maternity_list.push_back(per);
  report_birth(day, per);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp,"prepare to give birth: ");
    per->print(Global::Statusfp,0);
  }
}

void Population::setup() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "setup population entered\n");
    fflush(Global::Statusfp);
  }
  disease = new Disease [diseases];
  for (int dis = 0; dis < diseases; dis++) {
    disease[dis].setup(dis, this, mutation_prob[dis]);
  }
  
  Profile::read_profiles(Population::profilefile);
  vacc_manager = new Vaccine_Manager(this);
  av_manager   = new AV_Manager(this);
  if (Global::Verbose > 1) av_manager->print();
  pop.clear();
  pop_map.clear();
  pop_size = 0;
  maternity_list.clear();
  graveyard.clear();
  death_list.clear();
  read_population();

  // empty out the incremental list of Person's who have changed
  incremental_changes.clear();
  never_changed.clear();
  for (int p = 0; p < pop_size; p++){
    never_changed[pop[p]]=true; // add all agents to this list at start
  }

  if(Global::Verbose > 0){
    int count = 0;
    for(int p = 0; p < pop_size; p++){
      Disease* s = &disease[0];
      if(pop[p]->get_health()->is_immune(s)) count++;
    }
    fprintf(Global::Statusfp, "number of residually immune people = %d\n", count);
    fflush(Global::Statusfp);
  }
  av_manager->reset();
  vacc_manager->reset();
  
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "population setup finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::read_population() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "read population entered\n");
    fflush(Global::Statusfp);
  }
  
  // read in population
  char population_file[256];
  sprintf(population_file, "%s/%s", Global::Population_directory, Population::popfile);
  FILE *fp = fopen(population_file, "r");
  if (fp == NULL) {
    fprintf(Global::Statusfp, "population_file %s not found\n", population_file);
    exit(1);
  }
  int psize;
  if (1 != fscanf(fp, "Population = %d", &psize)){
    fprintf(Global::Statusfp, "failed to parse pop_size\n");
    exit(1);
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "Population = %d\n", psize);
    fflush(Global::Statusfp);
  }
  
  // create strings for original individuals
  // pstring = new char* [psize];
  // for (int i = 0; i < psize; i++) pstring[i] = new char[256];

  // reserve population vector
  pop.reserve(psize);
	
  // skip header line
  int skip = fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s");
  if (Global::Verbose > 99) {
    fprintf(Global::Statusfp, "skip = %d\n", skip);
  }

  Population::next_id = 0;
  for (int p = 0; p < psize; p++) {
    int age, married, occ;
    char label[32], house[32], school[32], work[32];
    char sex;
    if (fscanf(fp, "%s %d %c %d %d %s %s %s",
               label, &age, &sex, &married, &occ, house, school, work) != 8) {
      Utils::fred_abort("Help! Read failure for new person %d\n", p); 
    }
    Person * person = new Person(next_id, label, age, sex, married, occ, house, school, work, this, 0);
    add_person(person);
    // sprintf(pstring[next_id], "%s %d %c %d %d %s %s %s", label, age, sex, married, occ, house, school, work);
    // printf("pstring[%d]: %s\n", next_id, pstring[next_id]);
    Population::next_id++;
  }
  fclose(fp);
  assert(pop_size == psize);
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "finished reading population = %d\n", pop_size);
    fflush(Global::Statusfp);
  }
}

void Population::begin_day(int day) {
  time_t start_timer, stop_timer;

  // clear lists of births and deaths
  if (Global::Enable_Deaths) death_list.clear();
  if (Global::Enable_Births) maternity_list.clear();

  if (Global::Enable_Mobility) {
    // update household mobility activity on July 1
    if (Date::match_pattern(day, "07-01-*")) {
      for (int p = 0; p < pop_size; p++) {
	pop[p]->update_household_mobility();
      }
    }
  }

  // update activity profiles on July 1
    if (Global::Enable_Aging && Date::match_pattern(day, "07-01-*")) {
    for (int p = 0; p < pop_size; p++) {
      pop[p]->update_activity_profile();
    }
  }

  // update everyone's demographics
  time(&start_timer);
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_demographics(day);
  }
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update_demographics took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  // update everyone's health status
  time(&start_timer);
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_health(day);
  }
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update_health took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  if (Global::Enable_Births) {
    // add the births to the population
    size_t births = maternity_list.size();
    for (size_t i = 0; i < births; i++) {
      Person * baby = maternity_list[i]->give_birth(day);
      add_person(baby);
      int age_lookup = maternity_list[i]->get_age();
      if (age_lookup > Demographics::MAX_AGE)
	age_lookup = Demographics::MAX_AGE;
      birth_count[age_lookup]++;
    }
    if (Global::Verbose > 0) {
      fprintf(Global::Statusfp, "births = %d\n", (int)births);
      fflush(Global::Statusfp);
    }
  }

  if (Global::Enable_Births) {
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
    if (Global::Verbose > 0) {
      fprintf(Global::Statusfp, "deaths = %d\n", (int)deaths);
      fflush(Global::Statusfp);
    }
  }

  // update travel decisions
  time(&start_timer);
  Travel::update_travel(day);
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update_travel took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  // update adult decisions
  time(&start_timer);
  for (int p = 0; p < pop_size; p++) {
    if (Global::ADULT_AGE <= pop[p]->get_age()) {
      pop[p]->update_behavior(day);
    }
  }
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update_behavior for adults took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  // update child decisions
  time(&start_timer);
  for (int p = 0; p < pop_size; p++) {
    if (pop[p]->get_age() < Global::ADULT_AGE) {
      pop[p]->update_behavior(day);
    }
  }
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update_behavior for children took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  // distribute vaccines
  time(&start_timer);
  vacc_manager->update(day);
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update vacc_manager took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  // distribute AVs
  time(&start_timer);
  av_manager->update(day);
  time(&stop_timer);
  fprintf(Global::Statusfp, "day %d update av_manager took %d seconds\n",
	  day, (int) (stop_timer-start_timer));
  fflush(Global::Statusfp);

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "population begin_day finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::get_visitors_to_infectious_places(int day) {

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "get_visitors_to_infectious_places entered\n");
    fflush(Global::Statusfp);
  }

  // find places visited by infectious agents
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_infectious_activities(day);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "update_infectious_activities finished\n");
    fflush(Global::Statusfp);
  }

  // add susceptibles to infectious places
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_susceptible_activities(day);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "update_susceptible_activities finished\n");
    fflush(Global::Statusfp);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "get_visitors_to_infectious_places finished\n");
    fflush(Global::Statusfp);
  }

}

void Population::transmit_infection(int day) {

  // apply transmission model in all infectious places
  for (int s = 0; s < diseases; s++) {
    if (Global::Verbose > 1) {
      fprintf(Global::Statusfp, "disease = %d day = %d\n",s,day);
      fflush(Global::Statusfp);
      disease[s].print();
    }
    disease[s].update(day);
  }

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "find_infectious_places finished\n");
    fflush(Global::Statusfp);
  }

}

void Population::end_day(int day) {

  // give out anti-virals (after today's infections)
  av_manager->disseminate(day);

  if (Global::Verbose > 0 && Date::match_pattern(day, "12-31-*")) {
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
      fprintf(Global::Statusfp,
	      "DEMOGRAPHICS Year %d TotalPop %d Age %d ", 
	      Date::get_current_year(day), pop_size, i);
      count = age_count_female[i];
      num_births = birth_count[i];
      num_deaths = death_count_female[i];
      birthrate = count>0 ? ((100.0*num_births)/count) : 0.0;
      deathrate = count>0 ? ((100.0*num_deaths)/count) : 0.0;
      fprintf(Global::Statusfp,
	      "count_f %d births_f %d birthrate_f %.03f deaths_f %d deathrate_f %.03f ",
	      count, num_births, birthrate, num_deaths, deathrate);
      count = age_count_male[i];
      num_deaths = death_count_male[i];
      deathrate = count?((100.0*num_deaths)/count):0.0;
      fprintf(Global::Statusfp,
	      "count_m %d deaths_m %d deathrate_m %.03f\n",
	      count, num_deaths, deathrate);
      fflush(Global::Statusfp);
    }
    clear_static_arrays();
  }

  //Write the population to the output file if the parameter is set
  //  * Will write only on the first day of the simulation, days matching the date pattern in the parameter file,
  //    and the last day of the simulation *
  if(Population::output_population > 0) {
    if((day == 0) || (Date::match_pattern(day, Population::output_population_date_match))) {
      this->write_population_output_file(day);
    }
  }

  if(Global::Verbose > 1) {
    fprintf(Global::Statusfp, "population update finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::report(int day) {
  for (int s = 0; s < diseases; s++) {
    disease[s].update_stats(day);
    disease[s].print_stats(day);
  }
}

void Population::print(int incremental, int day) {
  if (Global::Tracefp == NULL) return;

  if (!incremental){
    if (Global::Trace_Headers) fprintf(Global::Tracefp, "# All agents, by ID\n");
    for (int p = 0; p < pop_size; p++)
      for (int i=0; i<diseases; i++)
	pop[p]->print(Global::Tracefp, i);
  } else if (1==incremental) {
    ChangeMap::const_iterator iter;
		
    if (Global::Trace_Headers){
      if (day < Global::Days)
	fprintf(Global::Tracefp, "# Incremental Changes (every %d): Day %3d\n", Global::Incremental_Trace, day);
      else
	fprintf(Global::Tracefp, "# End-of-simulation: Remaining unreported changes\n");
			
      if (! incremental_changes.size()){
	fprintf(Global::Tracefp, "# <LIST-EMPTY>\n");
	return;
      }
    }
		
    for (iter = this->incremental_changes.begin();
         iter != this->incremental_changes.end();
         iter++){
      (iter->first)->print(Global::Tracefp, 0); // the map key is a Person*
    }
  } else {
    ChangeMap::const_iterator iter;
    if (Global::Trace_Headers){
      fprintf(Global::Tracefp, "# Agents that never changed\n");
      if (! never_changed.size()){
	fprintf(Global::Tracefp, "# <LIST-EMPTY>\n");
	return;
      }
    }
		
    for (iter = this->never_changed.begin();
         iter != this->never_changed.end();
         iter++){
      (iter->first)->print(Global::Tracefp, 0); // the map key is a Person*
    }
  }
	
  // empty out the incremental list of Person's who have changed
  if (-1 < incremental)
    incremental_changes.clear();
}


void Population::end_of_run() {
  // print out agents who have changes yet unreported
  this->print(1, Global::Days);
  
  // print out all those agents who never changed
  this->print(-1);

  //Write the population to the output file if the parameter is set
  //  * Will write only on the first day of the simulation, days matching the date pattern in the parameter file,
  //    and the last day of the simulation *
  if(Population::output_population > 0) {
    this->write_population_output_file(Global::Days);
  }
}



Disease *Population::get_disease(int s) {
  return &disease[s];
}

void Population::quality_control() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "population quality control check\n");
    fflush(Global::Statusfp);
  }
  
  // check population
  for (int p = 0; p < pop_size; p++) {
    if (pop[p]->get_activities()->get_household() == NULL) {
      fprintf(Global::Statusfp, "Help! Person %d has no home.\n",p);
      pop[p]->print(Global::Statusfp, 0);
    }
  }
  
  if (Global::Verbose > 0) {
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
    fprintf(Global::Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 10; c++) {
      fprintf(Global::Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
              10*c, 10*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
    
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
        fprintf(Global::Statusfp, "\n Age Distribution of At Risk for Disease %d: %d people\n",is,total);
        for(int c = 0; c < 10; c++ ) {
          fprintf(Global::Statusfp, "age %2d to %2d: %6d (%.2f%%)\n",
                  10*c, 10*(c+1)-1, rcount[c], (100.0*rcount[c])/total);
        }
        fprintf(Global::Statusfp, "\n");
      }
    }	
  }
  
  
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "population quality control finished\n");
    fflush(Global::Statusfp);
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

int Population::get_next_id() {
  return Population::next_id++;
}

void Population::assign_classrooms() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign classrooms entered\n");
    fflush(Global::Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    pop[p]->assign_classroom();
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign classrooms finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::assign_offices() {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign offices entered\n");
    fflush(Global::Statusfp);
  }
  for (int p = 0; p < pop_size; p++){
    pop[p]->assign_office();
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "assign offices finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::get_network_stats(char *directory) {
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "get_network_stats entered\n");
    fflush(Global::Statusfp);
  }
  char filename[256];
  sprintf(filename, "%s/degree.txt", directory);
  FILE *fp = fopen(filename, "w");
  for (int p = 0; p < pop_size; p++){
    fprintf(fp, "%d %d %d\n",
      pop[p]->get_id(),
      pop[p]->get_age(),
      pop[p]->get_degree());
  }
  fclose(fp);
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "get_network_stats finished\n");
    fflush(Global::Statusfp);
  }
}

void Population::report_birth(int day, Person *per) const {
  if (Global::Birthfp == NULL) return;
  fprintf(Global::Birthfp, "day %d mother %d age %d\n",
	  day,
	  per->get_id(),
	  per->get_age());
  fflush(Global::Birthfp);
}

void Population::report_death(int day, Person *per) const {
  if (Global::Deathfp == NULL) return;
  fprintf(Global::Deathfp, "day %d person %d age %d\n",
	  day,
	  per->get_id(),
	  per->get_age());
  fflush(Global::Deathfp);
}

char * Population::get_pstring(int id) {
  return pstring[id];
}

void Population::print_age_distribution(char * dir, char * date_string, int run) {
  FILE *fp;
  int count[21];
  double pct[21];
  char filename[256];
  sprintf(filename, "%s/age_dist_%s.%02d", dir, date_string, run);
  printf("print_age_dist entered, filename = %s\n", filename); fflush(stdout);
  for (int i = 0; i < 21; i++) {
    count[i] = 0;
  }
  for (int p = 0; p < pop_size; p++){
    int age = pop[p]->get_age();
    int bin = age/5;
    if (-1 < bin && bin < 21) count[bin]++;
    if (-1 < bin && bin > 20) count[20]++;
  }
  fp = fopen(filename, "w");
  for (int i = 0; i < 21; i++) {
    pct[i] = 100.0*count[i]/pop_size;
    fprintf(fp, "%d  %d %f\n", i*5, count[i], pct[i]);
  }
  fclose(fp);
}

Person * Population::select_random_person() {
  int i = IRAND(0,pop_size-1);
  return pop[i];
}

void Population::write_population_output_file(int day) {

  //Loop over the whole population and write the output of each Person's to_string to the file
  char population_output_file[256];
  sprintf(population_output_file, "%s/%s.%s", Global::Output_directory,
                Global::Sim_Date->get_YYYYMMDD(day), Population::pop_outfile);
  FILE *fp = fopen(population_output_file, "w");
  if (fp == NULL) {
    Utils::fred_abort("Help! population_output_file %s not found\n", population_output_file);
  }

  fprintf(fp, "Population for day %d\n", day);
  fprintf(fp, "------------------------------------------------------------------\n");
  for (int p = 0; p < pop_size; ++p) {
    fprintf(fp, "%s\n", pop[p]->to_string().c_str());
  }
  fflush(fp);
  fclose(fp);
}

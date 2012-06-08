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
#include "Global.h"
#include "Place_List.h"
#include "Household.h"
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
#include "Activities.h"

using namespace std; 

char ** pstring;

// used for reporting
int age_count_male[Demographics::MAX_AGE + 1];
int age_count_female[Demographics::MAX_AGE + 1];
int birth_count[Demographics::MAX_AGE + 1];
int death_count_male[Demographics::MAX_AGE + 1];
int death_count_female[Demographics::MAX_AGE + 1];

char Population::popfile[256];
char Population::pop_outfile[256];
char Population::output_population_date_match[256];
int Population::output_population = 0;
bool Population::is_initialized = false;
int Population::next_id = 0;

Population::Population() {
  clear_static_arrays();
  pop.clear();
  pop_map.clear();
  birthday_map.clear();
  pop_size = 0;
  disease = NULL;
  av_manager = NULL;
  vacc_manager = NULL;
  mutation_prob = NULL;

  for(int i = 0; i < 367; ++i) {
	birthday_vecs[i].clear();
  }
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
  Params::get_param_from_string("popfile", Population::popfile);

  int num_mutation_params =
    Params::get_param_matrix((char *) "mutation_prob", &mutation_prob);
  if (num_mutation_params != Global::Diseases) {
    fprintf(Global::Statusfp,
        "Improper mutation matrix: expected square matrix of %i rows, found %i",
        Global::Diseases, num_mutation_params);
    exit(1);
  }

  if (Global::Verbose > 1) {
    printf("\nmutation_prob:\n");
    for (int i  = 0; i < Global::Diseases; i++)  {
      for (int j  = 0; j < Global::Diseases; j++) {
        printf("%f ", mutation_prob[i][j]);
      }
      printf("\n");
    }
  }

  //Only do this one time
  if(!Population::is_initialized) {
    Params::get_param_from_string("output_population", &Population::output_population);
    if(Population::output_population > 0) {
      Params::get_param_from_string("pop_outfile", Population::pop_outfile);
      Params::get_param_from_string("output_population_date_match", Population::output_population_date_match);
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

  //Put the person into the correct birthday list
  if(Global::Enable_Aging) {
	int pos = person->get_demographics()->get_birthdate()->get_day_of_year();
	//Check to see if the day of the year is after FEB 28
	if(pos > 59 && !Date::is_leap_year(person->get_demographics()->get_birthdate()->get_year()))
	  pos++;

	this->birthday_vecs[pos].push_back(person);
	this->birthday_map[person] = ((int)this->birthday_vecs[pos].size() - 1);
  }
}

void Population::delete_person(Person * person) {
  map<Person *,int>::iterator it;
  Utils::fred_verbose(1,"DELETE PERSON: %d\n", person->get_id());
  it = pop_map.find(person);
  if (it == pop_map.end()) {
    Utils::fred_verbose(0,"Help! person %d deleted, but not in the pop_map\n", person->get_id());
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
    Utils::fred_verbose(0,"pop_size = %d  pop.size() = %d\n",
        pop_size, (int) pop.size());
  }
  assert((unsigned) pop_size == pop.size());

  person->terminate();
  Utils::fred_verbose(1,"DELETED PERSON: %d\n", person->get_id());
  if ( Global::Enable_Travel ) {
    Travel::terminate_person(person);
  }
  delete person;
  // graveyard.push_back(person);
}

void Population::prepare_to_die(int day, Person *per) { // aka, 'bring out your dead'
  // add person to daily death_list
  death_list.push_back(per);
  report_death(day, per);
  // you'll be stone dead in a moment...
  per->die();
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
  disease = new Disease [Global::Diseases];
  for (int d = 0; d < Global::Diseases; d++) {
    disease[d].setup(d, this, mutation_prob[d]);
  }

  if ( Global::Enable_Vaccination ) {
    vacc_manager = new Vaccine_Manager(this);
  } else {
    vacc_manager = new Vaccine_Manager();
  }

  if ( Global::Enable_Antivirals ) {
    av_manager = new AV_Manager(this);
  } else {
    av_manager = new AV_Manager();
  }

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
  /*
     for (int p = 0; p < pop_size; p++){
     never_changed[pop[p]]=true; // add all agents to this list at start
     }
     */

  if(Global::Verbose > 0){
    int count = 0;
    for(int p = 0; p < pop_size; p++){
      Disease* d = &disease[0];
      if(pop[p]->get_health()->is_immune(d)) count++;
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
  char line[256];
  bool use_gzip = false;
  sprintf(population_file, "%s", Population::popfile);
  FILE *fp = Utils::fred_open_file(population_file);
  if (fp == NULL) {
    // try to find the gzipped version
    char population_file_gz[256];
    sprintf(population_file_gz, "%s.gz", Population::popfile);
    if (Utils::fred_open_file(population_file_gz)) {
      char cmd[256];
      use_gzip = true;
      sprintf(cmd, "gunzip -c %s > %s", population_file_gz, population_file);
      system(cmd);
      fp = Utils::fred_open_file(population_file);
    }
    // gunzip didn't work ...
    if (fp == NULL) {
      Utils::fred_abort("population_file %s not found\n", population_file);
    }
  }

  // create strings for original individuals
  // pstring = new char* [psize];
  // for (int i = 0; i < psize; i++) pstring[i] = new char[256];

  Population::next_id = 0;
  while (fgets(line, 255, fp) != NULL) {
    int age, married, occ, relationship;
    char label[32], house_label[32], school_label[32], work_label[32];
    char sex;

    // skip white-space-only lines
    int i = 0;
    while (i < 255 && line[i] != '\0' && isspace(line[i])) i++;
    if (line[i] == '\0') continue;

    // skip comment lines
    if (line[0] == '#') continue;

    if (sscanf(line, "%s %d %c %d %d %s %s %s %d",
          label, &age, &sex, &married, &occ,
          house_label, school_label, work_label, &relationship) != 9) {
      // relationship = Global::HOUSEHOLDER;
      // if (sscanf(line, "%s %d %c %d %d %s %s %s",
      // label, &age, &sex, &married, &occ,
      // house_label, school_label, work_label) != 8) {
      Utils::fred_abort("Help! Bad format in input line when next_id = %d: %s\n", Population::next_id, line);
      //}
    }
    Place * house = Global::Places.get_place_from_label(house_label);
    if (house == NULL) {
      printf("WARNING: skipping person %s in %s --  no household found for label = %s\n",
          label, population_file, house_label);
      fflush(stdout);
      continue;
    }
    Place * work = Global::Places.get_place_from_label(work_label);
    if (strcmp(work_label,"-1")!=0 && work == NULL) {
      if (Global::Verbose > 0) {
        printf("WARNING: person %s in %s -- no workplace found for label = %s\n",
            label, population_file, work_label);
        fflush(stdout);
      }
      if (Global::Enable_Local_Workplace_Assignment) {
        work = Global::Places.get_random_workplace();
        if (Global::Verbose > 0) {
          if (work != NULL) {
            printf("WARNING: person %s assigned to workplace %s\n",
                label, work->get_label());
          }
          else {
            printf("WARNING: no workplace available for person %s\n",
                label);
          }
        }
        fflush(stdout);
      }
    }
    Place * school = Global::Places.get_place_from_label(school_label);
    if (strcmp(school_label,"-1")!=0 && school == NULL) {
      printf("WARNING: person %s in %s -- no school found for label = %s\n",
          label, population_file, school_label);
      fflush(stdout);
    }
    bool today_is_birthday = false;
    int day = 0;
    Person * person = new Person(Population::next_id, age, sex, married, relationship, occ,
        house, school, work, day, today_is_birthday);
    add_person(person);
    Population::next_id++;
  }
  fclose(fp);
  if (use_gzip) {
    // remove the uncompressed file
    unlink(population_file);
  }

  // select adult to make health decisions
  for (int p = 0; p < pop_size; p++) {
    pop[p]->select_adult_decision_maker(NULL);
  }

  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "finished reading population, pop_size = %d\n", pop_size);
    fflush(Global::Statusfp);
  }
}

void Population::update(int day) {

  // clear lists of births and deaths
  if (Global::Enable_Deaths) death_list.clear();
  if (Global::Enable_Births) maternity_list.clear();

  if (Global::Enable_Aging) {

	//Find out if we are currently in a leap year
	int year = Global::Sim_Start_Date->get_year(day);
	int day_of_year = Date::get_day_of_year(year, Global::Sim_Start_Date->get_month(day), Global::Sim_Start_Date->get_day_of_month(day));

	int bd_count = 0;
	size_t vec_size = 0;

	printf("day_of_year = [%d]\n", day_of_year);

	bool is_leap = Date::is_leap_year(year);

	if((is_leap && day_of_year == 60) || (!is_leap && day_of_year != 60)) {
	  vec_size = this->birthday_vecs[day_of_year].size();
      for (size_t p = 0; p < vec_size; p++) {
        this->birthday_vecs[day_of_year][p]->get_demographics()->birthday(day);
        bd_count++;
      }
	}

	//If we are NOT in a leap year, then we need to do all of the day 60 (feb 29) birthdays on day 61
	if(!is_leap && day_of_year == 61) {
	  vec_size = this->birthday_vecs[60].size();
	  for (size_t p = 0; p < vec_size; p++) {
		this->birthday_vecs[60][p]->get_demographics()->birthday(day);
		bd_count++;
	  }
	}

	printf("birthday count = [%d]\n", bd_count);
  }

  // update everyone's demographics
  if (Global::Enable_Births || Global::Enable_Deaths || Global::Enable_Aging) {
    for (int p = 0; p < pop_size; p++) {
      pop[p]->update_demographics(day);
    }
  }
  // Utils::fred_print_wall_time("day %d update_demographics", day);

  if (Global::Enable_Births) {
    // add the births to the population
    size_t births = maternity_list.size();
    for (size_t i = 0; i < births; i++) {
      Person * mother = maternity_list[i];
      Person * baby = mother->give_birth(day);
      add_person(baby);

      if (Global::Enable_Behaviors) {
        // turn mother into an adult decision maker, if not already
        if (mother != mother->get_adult_decision_maker()) {
          Utils::fred_verbose(0, "young mother %d age %d become adult decision maker on day %d\n",
              mother->get_id(), mother->get_age(), day);
          mother->become_an_adult_decision_maker();
        }
        // let mother decide health behaviors for child
        baby->get_behavior()->set_adult_decision_maker(mother);
      }

      if(vacc_manager->do_vaccination()){
        if(Global::Debug > 1)
          fprintf(Global::Statusfp,"Adding %d to Vaccine Queue\n",baby->get_id());
        vacc_manager->add_to_queue(baby);
      }
      int age_lookup = mother->get_age();
      if (age_lookup > Demographics::MAX_AGE)
        age_lookup = Demographics::MAX_AGE;
      birth_count[age_lookup]++;
    }
    if (Global::Verbose > 0) {
      fprintf(Global::Statusfp, "births = %d\n", (int)births);
      fflush(Global::Statusfp);
    }
  }

  if (Global::Enable_Deaths) {
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

      if(vacc_manager->do_vaccination()){
        if(Global::Debug > 1)
          fprintf(Global::Statusfp,"Removing %d from Vaccine Queue\n", death_list[i]->get_id());
        vacc_manager->remove_from_queue(death_list[i]);
      }

      //Remove the person from the birthday lists
      if(Global::Enable_Aging) {
    	map<Person *, int>::iterator itr;
    	itr = birthday_map.find(death_list[i]);
        if (itr == birthday_map.end()) {
          Utils::fred_verbose(0,"Help! person %d deleted, but not in the birthday_map\n", death_list[i]->get_id());
        }
        assert(itr != birthday_map.end());
        int pos = (*itr).second;
        int day_of_year = death_list[i]->get_demographics()->get_birthdate()->get_day_of_year();

    	//Check to see if the day of the year is after FEB 28
    	if(day_of_year > 59 && !Date::is_leap_year(death_list[i]->get_demographics()->get_birthdate()->get_year()))
    	  day_of_year++;

        Person * last = this->birthday_vecs[day_of_year].back();
        birthday_map.erase(itr);
        birthday_map[last] = pos;

        this->birthday_vecs[day_of_year][pos] = this->birthday_vecs[day_of_year].back();
        this->birthday_vecs[day_of_year].pop_back();
      }

      delete_person(death_list[i]);
    }
    if (Global::Verbose > 0) {
      fprintf(Global::Statusfp, "deaths = %d\n", (int)deaths);
      fflush(Global::Statusfp);
    }
  }

  // first update everyone's health intervention status
  if ( Global::Enable_Vaccination || Global::Enable_Antivirals ) {
    // update everyone's health vaccination and antiviral status
    for (int p = 0; p < pop_size; p++) {
      pop[p]->update_health_interventions(day);
    }
  }

  // update everyone's health status
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_health(day);
  }
  // Utils::fred_print_wall_time("day %d update_health", day);

  if (Global::Enable_Mobility) {
    // update household mobility activity on July 1
    if (Date::match_pattern(Global::Sim_Current_Date, "07-01-*")) {
      for (int p = 0; p < pop_size; p++) {
        pop[p]->update_household_mobility();
      }
    }
  }

  // prepare Activities at start up
  if (day == 0) {
    for (int p = 0; p < pop_size; p++) {
      pop[p]->prepare_activities();
    }
    Activities::before_run();
  }

  // update activity profiles on July 1
  if (Global::Enable_Aging && Date::match_pattern(Global::Sim_Current_Date, "07-01-*")) {
    for (int p = 0; p < pop_size; p++) {
      pop[p]->update_activity_profile();
    }
  }

  // update travel decisions
  Travel::update_travel(day);
  // Utils::fred_print_wall_time("day %d update_travel", day);

  // update decisions about behaviors
  for (int p = 0; p < pop_size; p++) {
    pop[p]->update_behavior(day);
  }
  // Utils::fred_print_wall_time("day %d update_behavior", day);

  // distribute vaccines
  vacc_manager->update(day);
  // Utils::fred_print_wall_time("day %d vacc_manager", day);

  // distribute AVs
  av_manager->update(day);
  // Utils::fred_print_wall_time("day %d av_manager", day);

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "population begin_day finished\n");
    fflush(Global::Statusfp);
  }

}

void Population::report(int day) {

  // give out anti-virals (after today's infections)
  av_manager->disseminate(day);

  if (Global::Verbose > 0 && Date::match_pattern(Global::Sim_Current_Date, "12-31-*")) {
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
          Global::Sim_Current_Date->get_year(), pop_size, i);
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

  for (int d = 0; d < Global::Diseases; d++) {
    disease[d].print_stats(day);
  }

  // Write out the population if the output_population parameter is set.
  // Will write only on the first day of the simulation, on days
  // matching the date pattern in the parameter file, and the on
  // the last day of the simulation
  if(Population::output_population > 0) {
    if((day == 0) || (Date::match_pattern(Global::Sim_Current_Date, Population::output_population_date_match))) {
      this->write_population_output_file(day);
    }
  }

}

void Population::print(int incremental, int day) {
  if (Global::Tracefp == NULL) return;

  if (!incremental){
    if (Global::Trace_Headers) fprintf(Global::Tracefp, "# All agents, by ID\n");
    for (int p = 0; p < pop_size; p++)
      for (int i=0; i<Global::Diseases; i++)
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

Disease *Population::get_disease(int disease_id) {
  return &disease[disease_id];
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
    int n0, n5, n18, n65;
    int count[20];
    int total = 0;
    n0 = n5 = n18 = n65 = 0;
    // age distribution
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < pop_size; p++) {
      int a = pop[p]->get_age();
      if (a < 5) { n0++; }
      else if (a < 18) { n5++; }
      else if (a < 65) { n18++; }
      else { n65++; }
      int n = a / 5;
      if (n < 20) { count[n]++; }
      else { count[19]++; }
      total++;
    }
    fprintf(Global::Statusfp, "\nAge distribution: %d people\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Global::Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
          5*c, 5*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "AGE 0-4: %d %.2f%%\n", n0, (100.0*n0)/total);
    fprintf(Global::Statusfp, "AGE 5-17: %d %.2f%%\n", n5, (100.0*n5)/total);
    fprintf(Global::Statusfp, "AGE 18-64: %d %.2f%%\n", n18, (100.0*n18)/total);
    fprintf(Global::Statusfp, "AGE 65-100: %d %.2f%%\n", n65, (100.0*n65)/total);
    fprintf(Global::Statusfp, "\n");

    // Print out At Risk distribution
    for(int d = 0; d < Global::Diseases; d++){
      if(disease[d].get_at_risk()->get_num_ages() > 0){
        Disease* dis = &disease[d];
        int rcount[20];
        for (int c = 0; c < 20; c++) { rcount[c] = 0; }
        for (int p = 0; p < pop_size; p++) {
          int a = pop[p]->get_age();
          int n = a / 10;
          if(pop[p]->get_health()->is_at_risk(dis)==true) {
            if( n < 20 ) { rcount[n]++; }
            else { rcount[19]++; }
          }
        }
        fprintf(Global::Statusfp, "\n Age Distribution of At Risk for Disease %d: %d people\n",d,total);
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
  sprintf(filename, "%s/degree.csv", directory);
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "id,age,deg,h,n,s,c,w,o\n");
  for (int p = 0; p < pop_size; p++){
    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
        pop[p]->get_id(),
        pop[p]->get_age(),
        pop[p]->get_degree(),
        pop[p]->get_household_size(),
        pop[p]->get_neighborhood_size(),
        pop[p]->get_school_size(),
        pop[p]->get_classroom_size(),
        pop[p]->get_workplace_size(),
        pop[p]->get_office_size());
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

Person * Population::select_random_person_by_age(int min_age, int max_age) {
  int i = IRAND(0,pop_size-1);
  while (pop[i]->get_age() < min_age || pop[i]->get_age() > max_age) {
    i = IRAND(0,pop_size-1);
  }
  return pop[i];
}

void Population::write_population_output_file(int day) {

  //Loop over the whole population and write the output of each Person's to_string to the file
  char population_output_file[256];
  sprintf(population_output_file, "%s/%s_%s.txt", Global::Output_directory, Population::pop_outfile,
      (char *) Global::Sim_Current_Date->get_YYYYMMDD().c_str());
  FILE *fp = fopen(population_output_file, "w");
  if (fp == NULL) {
    Utils::fred_abort("Help! population_output_file %s not found\n", population_output_file);
  }

  //  fprintf(fp, "Population for day %d\n", day);
  //  fprintf(fp, "------------------------------------------------------------------\n");
  for (int p = 0; p < pop_size; ++p) {
    fprintf(fp, "%s\n", pop[p]->to_string().c_str());
  }
  fflush(fp);
  fclose(fp);
}

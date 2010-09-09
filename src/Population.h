/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Population.h
//

#ifndef _FRED_POPULATION_H
#define _FRED_POPULATION_H

#include "Global.h"
#include <vector>

class Person;
class Strain;
class Antivirals;
class AV_Manager;
class Vaccine_Manager;
class Age_Map;

using namespace std;
#include <map>
typedef map <Person*, bool> ChangeMap;	

class Population {
public:
  Population();
  UNIT_TEST_VIRTUAL ~Population();
  UNIT_TEST_VIRTUAL void get_parameters();
  UNIT_TEST_VIRTUAL void setup();
  UNIT_TEST_VIRTUAL void population_quality_control();
  UNIT_TEST_VIRTUAL void print(int incremental=0, int day=0); // 0:print all, 1:incremental, -1:unchanged
  UNIT_TEST_VIRTUAL void end_of_run();
  UNIT_TEST_VIRTUAL void reset(int run);
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL void report(int day);
  UNIT_TEST_VIRTUAL Strain * get_strain(int s);
  UNIT_TEST_VIRTUAL int get_strains() { return strains; }
  UNIT_TEST_VIRTUAL Person **get_pop() { return pop; }
  UNIT_TEST_VIRTUAL int get_pop_size() { return pop_size; }
  UNIT_TEST_VIRTUAL Age_Map* get_pregnancy_prob() { return pregnancy_prob; }
  //Mitigation Managers
  UNIT_TEST_VIRTUAL AV_Manager *get_av_manager(){ return av_manager; }
  UNIT_TEST_VIRTUAL Vaccine_Manager *get_vaccine_manager() { return vacc_manager;}
  
  // Modifiers on the entire pop;
  // void apply_residual_immunity(Strain *strain) {}
  
  // track those agents that have changed since the last incremental dump
  UNIT_TEST_VIRTUAL void set_changed(Person *p);
  
private:
  char popfile[256];
  char profilefile[256];
  Person **pop;					// list of all agents
  vector<Person *> children;		  // list of children (under 18)
  vector<Person *> adults;		 // list of adults (18 or older)
  ChangeMap incremental_changes; // incremental "list" (actually a C++ map)
				 // of those agents whose stats
				 // have changed since the last history dump
  ChangeMap never_changed;       // agents who have *never* changed
  int pop_size;
  Strain *strain;
  
  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;
  
  int strains;
  double **mutation_prob;
  void read_population();
	
  // Population specific Age Maps			
  Age_Map* pregnancy_prob;
};

#endif // _FRED_POPULATION_H

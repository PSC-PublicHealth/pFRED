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
#include "Person_Event_Interface.h"
#include "Demographics.h"

class Person;
class Disease;
class Antivirals;
class AV_Manager;
class Vaccine_Manager;

using namespace std;
#include <map>
#include <vector>
typedef map <Person*, bool> ChangeMap;	

class Population : public Person_Event_Interface {
public:
  Population();
  UNIT_TEST_VIRTUAL ~Population();
  UNIT_TEST_VIRTUAL void get_parameters();
  UNIT_TEST_VIRTUAL void setup();
  UNIT_TEST_VIRTUAL void quality_control();
  UNIT_TEST_VIRTUAL void print(int incremental=0, int day=0); // 0:print all, 1:incremental, -1:unchanged
  UNIT_TEST_VIRTUAL void end_of_run();
  void begin_day(int day);
  void get_visitors_to_infectious_places(int day);
  void transmit_infection(int day);
  void end_day(int day);
  UNIT_TEST_VIRTUAL void report(int day);
  UNIT_TEST_VIRTUAL Disease * get_disease(int s);
  UNIT_TEST_VIRTUAL int get_diseases() { return diseases; }
  UNIT_TEST_VIRTUAL int get_pop_size() { return pop_size; }

  //Mitigation Managers
  UNIT_TEST_VIRTUAL AV_Manager *get_av_manager(){ return av_manager; }
  UNIT_TEST_VIRTUAL Vaccine_Manager *get_vaccine_manager() { return vacc_manager;}
  void add_person(Person * per);
  void delete_person(Person * per);
  void prepare_to_die(int day, Person *per);
  void prepare_to_give_birth(int day, Person *per);
  Person *get_person(int n) { return pop[n]; }
  
  // Modifiers on the entire pop;
  // void apply_residual_immunity(Disease *disease) {}

  //Implement the interface
  UNIT_TEST_VIRTUAL void handle_property_change_event(Person *source, string property_name, int prev_val, int new_val);
  UNIT_TEST_VIRTUAL void handle_property_change_event(Person *source, string property_name, bool new_val);

  // track those agents that have changed since the last incremental dump
  UNIT_TEST_VIRTUAL void set_changed(Person *p);
  
  int get_next_id();
  void assign_classrooms();
  void assign_offices();
  void read_population();
  void report_birth(int day, Person *per) const;
  void report_death(int day, Person *per) const;

private:
  char popfile[256];
  char profilefile[256];
  vector <Person *> pop;			// list of all agents
  vector <Person *> graveyard;		      // list of all dead agents
  vector <Person *> death_list;		     // list agents to die today
  vector <Person *> maternity_list;   // list agents to give birth today
  int next_id;
  int pop_size;
  int diseases;
  Disease *disease;
  double **mutation_prob;
  map<Person *,int> pop_map;
  ChangeMap incremental_changes; // incremental "list" (actually a C++ map)
				 // of those agents whose stats
				 // have changed since the last history dump
  ChangeMap never_changed;       // agents who have *never* changed

  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;

  //Used for reporting
  void clear_static_arrays();

};

#endif // _FRED_POPULATION_H

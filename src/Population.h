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

class Population {
public:

  /**
   * Default constructor
   */
  Population();
  ~Population();

  /**
   * Sets the static variables for the class from the parameter file.
   */
  void get_parameters();

  /**
   * Prepare this Population (calls read_population)
   * @see Population::read_population()
   */
  void setup();

  /**
   * Used during debugging to verify that code is functioning properly.
   */
  void quality_control();

  /**
   * Print out information about this object.  This print can be adjusted to limit the output to just those
   * agents who have changed status (0:print all, 1:incremental, -1:unchanged) with the incremental flag.
   * @param incremental the flag to adjust the agents who are actually printed out
   * @param day the simulation day
   */
  void print(int incremental=0, int day=0);

  /**
   * Perform end of run operations (clean up)
   */
  void end_of_run();

  /**
   * Perform beginning of day operations
   * @param day the simulation day
   */
  void update(int day);

  /**
   * Report the disease statistics for a given day
   * @param day the simulation day
   */
  void report(int day);

  /**
   * @param disease_id the index of the Disease
   * @return a pointer to the Disease indexed by s
   */
  Disease * get_disease(int disease_id);

  /**
   * @return the pop_size
   */
  int get_pop_size() { return pop_size; }

  //Mitigation Managers
  /**
   * @return a pointer to this Population's AV_Manager
   */
  AV_Manager * get_av_manager(){ return av_manager; }

  /**
   * @return a pointer to this Population's Vaccine_Manager
   */
  Vaccine_Manager * get_vaccine_manager() { return vacc_manager;}

  /**
   * @param per a pointer to the Person to add to the Population
   */
  void add_person(Person * per);

  /**
   * @param per a pointer to the Person to remove from the Population
   */
  void delete_person(Person * per);

  /**
   * Perform the necessary steps for an agent's death
   * @param day the simulation day
   * @param per the agent who will die
   */
  void prepare_to_die(int day, Person *per);

  /**
   * Perform the necessary steps for an agent to give birth
   * @param day the simulation day
   * @param per the agent who will give birth
   */
  void prepare_to_give_birth(int day, Person *per);
  
  /**
   * @param n the index of the Person
   * Return a pointer to the Person object with index n
   */
  Person * get_person(int n) { return pop[n]; }

  // Modifiers on the entire pop;
  // void apply_residual_immunity(Disease *disease) {}

  // track those agents that have changed since the last incremental dump
  /**
   * Sets the changed flag for a Person so that the incremental print option will know that the agent has indeed changed
   * @param p pointer to the Person object that has changed
   */
  void set_changed(Person *p);

  /**
   * Assign agents in Schools to specific Classrooms within the school
   */
  void assign_classrooms();
  
  /**
   * Assign agents in Workplaces to specific Offices within the workplace
   */
  void assign_offices();

  /**
   * Write degree information to a file degree.txt
   * @param directory the directory where the file will be written
   */
  void get_network_stats(char *directory);

  /**
   * Read the population from the population file
   */
  void read_population();

  /**
   * Print the birth information to the status file
   * @see Global::Birthfp
   * @param day the simulation day
   * @param per a pointer to the Person object that has given birth
   */
  void report_birth(int day, Person *per) const;

  /**
   * Print the death information to the status file
   * @see Global::Deathfp
   * @param day the simulation day
   * @param per a pointer to the Person object that has died
   */
  void report_death(int day, Person *per) const;

  /**
   * @param id the id of the Person
   * @return the population string used during setup that matches the id
   */
  char * get_pstring(int id);

  /**
   *
   */
  void print_age_distribution(char *dir, char * date_string, int run);

  /**
   * @return a pointer to a random Person in this population
   */
  Person * select_random_person();

  /**
   * @return a pointer to a random Person in this population
   * whose age is in the given range
   */
  Person * select_random_person_by_age(int min_age, int max_age);

  /**
   * Static function to get and increment the next_id
   * @return the next id value
   */
  static int get_next_id();

private:
  vector <Person *> pop;			// list of all agents
  vector <Person *> graveyard;		      // list of all dead agents
  vector <Person *> death_list;		     // list agents to die today
  vector <Person *> maternity_list;   // list agents to give birth today
  int pop_size;
  Disease *disease;
  double **mutation_prob;
  map<Person *,int> pop_map;
  ChangeMap incremental_changes; // incremental "list" (actually a C++ map)
				 // of those agents whose stats
				 // have changed since the last history dump
  ChangeMap never_changed;       // agents who have *never* changed

  static char popfile[256];
  static char profilefile[256];
  static char pop_outfile[256];
  static char output_population_date_match[256];
  static int output_population;
  static bool is_initialized;
  static int next_id;

  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;

  /**
   * Used for reporting
   */
  void clear_static_arrays();

  /**
   * Write out the population in a format similar to the population input files (with additional runtime information)
   * @param day the simulation day
   */
  void write_population_output_file(int day);

};

#endif // _FRED_POPULATION_H

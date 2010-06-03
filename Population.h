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
  Population(void);
  ~Population(void);
  void get_parameters();
  void setup();
  void population_quality_control();
  void print(int incremental=0, int day=0); // 0:print all, 1:incremental, -1:unchanged
  void end_of_run();
  void reset(int run);
  void update(int day);
  void report(int day);
  Strain * get_strain(int s);
  int get_strains() { return strains; }
  Person *get_pop() { return pop; }
  int get_pop_size() { return pop_size; }
  Age_Map* get_pregnancy_prob(void) { return pregnancy_prob; }
  //Mitigation Managers
  AV_Manager *get_av_manager(){ return av_manager; }
  Vaccine_Manager *get_vaccine_manager() { return vacc_manager;}

  // Modifiers on the entire pop;
  void apply_residual_immunity(Strain strain);

  // track those agents that have changed since the last incremental dump
  void set_changed(Person *p);

private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  ChangeMap incremental_changes; // incremental "list" (actually a C++ map)
                                 // of those agents whose stats
                                 // have changed since the last history dump
  ChangeMap never_changed;       // agents who have *never* changed
  int pop_size;
  Strain *strain;
  
  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;
  //double vaccine_compliance;  // to be replaced by vaccine_manager

  int strains;
  double **mutation_prob;
  void read_population();

  // Population specific Age Maps
  Age_Map* pregnancy_prob;
};

#endif // _FRED_POPULATION_H

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Population.hpp
//

#ifndef _FRED_POPULATION_H
#define _FRED_POPULATION_H

#include <set>
#include <stack>
using namespace std;

class Person;

class Population {
public:
  void get_population_parameters();
  void setup_population();
  void read_population();
  void reset_population(int run);
  void population_quality_control();
  void update_exposed_population(int day);
  void update_infectious_population(int day);
  void update_population_behaviors(int day);
  void update_population_stats(int day);
  void print_population_stats(int day);
  void print_population();
  void insert_into_exposed_list(int d, int p);
  void insert_into_infectious_list(int d, int p);
  void remove_from_exposed_list(int d, int p);
  void remove_from_infectious_list(int d, int p);
  void start_outbreak();
  int get_age(int per);
  int get_role(int per, int strain);
  char get_strain_status(int per, int strain);
  int is_place_on_schedule_for_person(int per, int day, int loc);
  double get_infectivity(int per, int strain);
  double get_susceptibility(int per, int strain);
  void make_exposed(int per, int strain, int infector, int loc, char type, int day);
  void add_infectee(int per, int strain);
  void update_schedule(int per, int day);
  void get_schedule(int per, int *n, int *schedule);
  double get_attack_rate(int strain);
  set <int> *infectious;

private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  int pop_size;
  int index_cases;
  set <int> *exposed;
  int *S;
  int *E;
  int *I;
  int *R;
  double * attack_rate;
};

#endif // _FRED_POPULATION_H
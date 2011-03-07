/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Epidemic.h
//

#ifndef _FRED_EPIDEMIC_H
#define _FRED_EPIDEMIC_H

#include "Global.h"
#include <set>
#include <map>
#include <vector>

using namespace std;

class Disease;
class Person;
class Timestep_Map;
class Place;

class Epidemic {
public:
  Epidemic(Disease * str, Timestep_Map *);
  UNIT_TEST_VIRTUAL ~Epidemic();
  
  void reset();
  void update_stats(int day);
  void print_stats(int day);
  void add_infectious_place(Place *p, char type);
  void get_infectious_places(int day);
  void insert_into_infected_list(Person * person) { infected.push_back(person); }
  void insert_into_infectious_list(Person * person) { infectious.insert(person); }
  void remove_from_infectious_list(Person * person) { infectious.erase(person); }
  double get_attack_rate() { return attack_rate; }
  void update(int day);
  int get_clinical_incidents() { return clinical_incidents; }
  int get_incident_infections() { return incident_infections; }
  int get_total_incidents() { return total_incidents; }
  void increment_S_count() { S_count++; }
  void decrement_S_count() { S_count--; }
  void increment_E_count() { E_count++; }
  void decrement_E_count() { E_count--; }
  void increment_I_count() { I_count++; }
  void decrement_I_count() { I_count--; }
  void increment_i_count() { i_count++; }
  void decrement_i_count() { i_count--; }
  void increment_R_count() { R_count++; }
  void decrement_R_count() { R_count--; }
  void increment_r_count() { r_count++; }
  void decrement_r_count() { r_count--; }
  void increment_C_count() { C_count++; }
  void decrement_C_count() { C_count--; }
  void increment_c_count() { c_count++; }
  void decrement_c_count() { c_count--; }
  void increment_M_count() { M_count++; }
  void decrement_M_count() { M_count--; }
  
private:
  Disease * disease;
  int id;
  int N;				      // current population size
  int N_init;				      // initial population size
  vector <Place *> inf_households;
  vector <Place *> inf_neighborhoods;
  vector <Place *> inf_classrooms;
  vector <Place *> inf_schools;
  vector <Place *> inf_workplaces;
  vector <Place *> inf_offices;
  double attack_rate;
  double clinical_attack_rate;
  set <Person *> infectious;
  Timestep_Map* primary_cases_map;
  int clinical_incidents;
  int total_clinical_incidents;
  int incident_infections;
  int total_incidents;
  int vaccine_acceptance;
  int * new_cases;
  vector <Person *> infected;
  int r_index;
  double RR;
  int NR;
  int S_count;
  int E_count;
  int I_count;
  int i_count;
  int R_count;
  int r_count;
  int C_count;
  int c_count;
  int M_count;
};

#endif // _FRED_EPIDEMIC_H

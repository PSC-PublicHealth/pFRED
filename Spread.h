/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Spread.h
//

#ifndef _FRED_SPREAD_H
#define _FRED_SPREAD_H

#include "Global.h"
#include <set>
#include <map>
#include <vector>

using namespace std;

class Strain;
class Person;
class Timestep_Map;

class Spread {
 public:
  Spread(Strain * str, Timestep_Map *);
  UNIT_TEST_VIRTUAL ~Spread();
  
  void reset();
  void update_stats(int day);
  void print_stats(int day);
  void insert_into_exposed_list(Person * person);
  void insert_into_infectious_list(Person * person);
  void remove_from_exposed_list(Person * person);
  void remove_from_infectious_list(Person * person);
  bool is_in_exposed_list(Person *per);
  bool is_in_infectious_list(Person *per);
  double get_attack_rate() { return attack_rate; }
  void update(int day);
  int get_S() { return S; }
  int get_E() { return E; }
  int get_I() { return I; }
  int get_I_s() { return I_s; }
  int get_R() { return R; }
  int get_M() { return M; }
  int get_clinical_incidents() { return clinical_incidents; }
  int get_incident_infections() { return incident_infections; }
  int get_total_incidents() { return total_incidents; }
  
 private:
  Strain * strain;
  double attack_rate;
  double clinical_attack_rate;
  set <Person *> exposed;
  set <Person *> infectious;
  //map <int, int> primary_cases_map;
  Timestep_Map* primary_cases_map;
  int S;
  int E;
  int I;
  int I_s;
  int R;
  int M;
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
};

#endif // _FRED_SPREAD_H

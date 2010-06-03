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

#include <set>
#include <map>
#include <vector>

using namespace std;

class Strain;
class Person;
class Timestep_Map;

class Spread {
public:
  Spread(Strain * str);
  ~Spread(void);
  void reset();
  void update_stats(int day);
  void print_stats(int day);
  void insert_into_exposed_list(Person * person);
  void insert_into_infectious_list(Person * person);
  void remove_from_exposed_list(Person * person);
  void remove_from_infectious_list(Person * person);
  double get_attack_rate() { return attack_rate; }
  void update(int day);

private:
  Strain * strain;
  double attack_rate;
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
  int incident_infections;
  int total_incidents;
  int clinical_incidents;
};
  
#endif // _FRED_SPREAD_H

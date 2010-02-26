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

class Spread {
public:
  Spread(Strain * str);
  void reset();
  void start_outbreak(Person *pop, int pop_size);
  void update_stats(Person *pop, int pop_size, int day);
  void print_stats(int day);
  void insert_into_exposed_list(Person * person);
  void insert_into_infectious_list(Person * person);
  void remove_from_exposed_list(Person * person);
  void remove_from_infectious_list(Person * person);
  int get_index_cases() { return index_cases; }
  double get_attack_rate() { return attack_rate; }
  void update(int day);

private:
  Strain * strain;
  int index_cases;
  double attack_rate;
  set <Person *> exposed;
  set <Person *> infectious;
  vector <Person *> not_yet_exposed;
  map <int, int> new_cases_map;
  int new_cases;
  int S;
  int E;
  int I;
  int I_s;
  int R;
};
  
#endif // _FRED_SPREAD_H

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
class Strain;

class Population {
public:
  void get_population_parameters();
  void setup_population();
  void read_population();
  void reset_population(int run);
  void population_quality_control();
  void start_outbreak();
  void update(int day);
  void report(int day);
  void print_population();
  Strain * get_strain(int s);
  int get_strains() { return strains; }

private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  int pop_size;
  Strain *strain;
  int strains;
  void update_exposed_population(int day);
  void update_infectious_population(int day);
  void update_population_behaviors(int day);
  void update_population_stats(int day);
  void print_population_stats(int day);
};

#endif // _FRED_POPULATION_H

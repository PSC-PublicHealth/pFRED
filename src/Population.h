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
#include "Antivirals.h"

class Person;
class Strain;
class Antivirals;
class AgeMap;

class Population {
public:
  void get_parameters();
  void setup();
  void population_quality_control();
  void print();
  void start_outbreak();
  void reset(int run);
  void update(int day);
  void report(int day);
  Strain * get_strain(int s);
  int get_strains() { return strains; }
  Person *get_pop() { return pop; }
  int get_pop_size() { return pop_size; }
  Antivirals *get_AVs() { return &AVs; }
  
  // Modifiers on the entire pop;
  void apply_residual_immunity(Strain strain);
  
private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  int pop_size;
  Strain *strain;
  Antivirals AVs;
  int strains;
  double **mutation_prob;
  void read_population();
  AgeMap* residual_immunity;
};

#endif // _FRED_POPULATION_H

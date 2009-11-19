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

class Person;
class Strain;

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

private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  int pop_size;
  Strain *strain;
  int strains;
  void read_population();
};

#endif // _FRED_POPULATION_H

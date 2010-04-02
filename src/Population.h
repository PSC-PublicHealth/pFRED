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

class Population {
public:
  void get_parameters();
  void setup();
  void population_quality_control();
  void print();
  void end_of_run();
  void reset(int run);
  void update(int day);
  void report(int day);
  Strain * get_strain(int s);
  int get_strains() { return strains; }
  Person *get_pop() { return pop; }
  int get_pop_size() { return pop_size; }
  //Mitigation Managers
  AV_Manager *get_av_manager(){ return av_manager; }
  Vaccine_Manager *get_vaccine_manager() { return vacc_manager;}

  // Modifiers on the entire pop;
  void apply_residual_immunity(Strain strain);
  
private:
  char popfile[80];
  char profilefile[80];
  Person *pop;
  int pop_size;
  Strain *strain;
  
  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;
  //double vaccine_compliance;  // to be replaced by vaccine_manager

  int strains;
  double **mutation_prob;
  void read_population();
  Age_Map* residual_immunity;
};

#endif // _FRED_POPULATION_H

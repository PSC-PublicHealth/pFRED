/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.h
//

#ifndef _FRED_STRAIN_H
#define _FRED_STRAIN_H

#include <set>
#include <stack>
using namespace std;

class Infection;
class Person;
class Place;
class Population;
class Spread;

class Strain {
public:
  Strain();
  void reset();
  void setup(int s, Population *pop,  double *mut_prob);
  void print();
  void update(int day);

  // If the infectee is susceptible, is at the given place on the given day,
  // and if the random roll for transmissibility succeeds
  // the infection is successful and attempt_infection returns true.
  bool attempt_infection(Person* infector, Person* infectee, Place* place, int exposure_date);

  int get_days_latent();
  int get_days_incubating();
  int get_days_infectious();
  int get_symptoms();

  int get_id() { return id; }
  double get_transmissibility() { return transmissibility; }
  double get_prob_symptomatic() { return prob_symptomatic; }
  double get_prob_resistant() { return prob_resistant; }
  int get_index_cases();
  double get_attack_rate();
  
  // Draws from this strain's mutation probability distribution, and returns
  // the strain to mutate to, or NULL if no mutation should occur.
  Strain* should_mutate_to();

  void insert_into_exposed_list(Person *person);
  void insert_into_infectious_list(Person *person);
  void remove_from_exposed_list(Person *person);
  void remove_from_infectious_list(Person *person);
  void start_outbreak(Person *pop, int pop_size);
  void update_stats(Person *pop, int pop_size, int day);
  void print_stats(int day);

  static int draw_from_distribution(int n, double *dist);
  static double get_prob_stay_home();
  static void get_strain_parameters();

private:
  int id;
  double transmissibility;
  double prob_symptomatic;
  double prob_resistant;
  int max_days_latent;
  int max_days_incubating;
  int max_days_infectious;
  double *days_latent;
  double *days_incubating;
  double *days_infectious;
  double *mutation_prob;
  Spread *spread;

  // Vars that are not strain-specific (for updating global stats).
  Population *population;
};
  


#endif // _FRED_STRAIN_H

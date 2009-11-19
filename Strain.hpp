/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Strain.hpp
//

#ifndef _FRED_STRAIN_H
#define _FRED_STRAIN_H

#include <set>
using namespace std;
#include "Person.hpp"

class Strain {
public:
  Strain();
  void reset();
  void setup(int s);
  void print();
  int get_days_latent();
  int get_days_incubating();
  int get_days_infectious();
  int get_id() { return id; }
  double get_transmissibility() { return transmissibility; }
  double get_prob_symptomatic() { return prob_symptomatic; }
  double get_prob_resistant() { return prob_resistant; }
  int get_index_cases() { return index_cases; }
  double get_attack_rate() { return attack_rate; }
  void insert_into_exposed_list(Person * per);
  void insert_into_infectious_list(Person * per);
  void remove_from_exposed_list(Person * per);
  void remove_from_infectious_list(Person * per);
  static int draw_from_distribution(int n, double *dist);
  static double get_prob_stay_home();
  static void get_strain_parameters();
  void start_outbreak(Person *pop, int pop_size);
  void update_exposed(int day);
  void update_infectious(int day);
  void update_stats(Person *pop, int pop_size, int day);
  void print_stats(int day);



  set <Person *> infectious;

private:
  int id;
  double transmissibility;
  double prob_symptomatic;
  double prob_resistant;
  int index_cases;
  double attack_rate;
  int max_days_latent;
  int max_days_incubating;
  int max_days_infectious;
  double *days_latent;
  double *days_incubating;
  double *days_infectious;
  set <Person *> exposed;
  int S;
  int E;
  int I;
  int R;
};
  


#endif // _FRED_STRAIN_H

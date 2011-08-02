/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.h
//

#ifndef _FRED_Disease_H
#define _FRED_Disease_H

#include "Global.h"
#include "Epidemic.h"
#include <map>

using namespace std;

class Person;
class Population;
class Age_Map;
class StrainTable;
class IntraHost;
class Trajectory;
class Infection;

class Disease {
public:
  Disease();
  ~Disease();

  void setup(int s, Population *pop,  double *mut_prob);
  void print();
  void update(Date *sim_start_date, int day);

  // The methods draw from the underlying distributions to randomly determine some aspect of the infection
  // have been moved to the DefaultIntraHost class

  int get_days_symp();
  int get_days_recovered();
  int get_max_days();
  double get_mortality_rate() { return mortality_rate;}
  int get_id() { return id;}
  double get_transmissibility() { return transmissibility;}
  double get_transmissibility(int strain);
  double get_attack_rate();
  Age_Map* get_residual_immunity() const { return residual_immunity;}
  Age_Map* get_at_risk() const { return at_risk;}
  map<int, double> *getPrimaryLoads(int day);
  map<int, double> *getPrimaryLoads(int day, int strain);
  Evolution *get_evolution() { return evol;}

  Trajectory *getTrajectory(Infection *infection, map<int, double> *loads);

  // Draws from this Disease's mutation probability distribution, and returns
  // the Disease to mutate to, or NULL if no mutation should occur.
  Disease* should_mutate_to();

  void add_infectious_place(Place *p, char type);
  void insert_into_infected_list(Person *person);
  void insert_into_infectious_list(Person *person);
  void remove_from_infectious_list(Person *person);
  void update_stats(int day);
  void print_stats(int day);

  Population * get_population() { return population;}
  Epidemic * get_epidemic() { return epidemic;}

  static double get_prob_stay_home();
  static void set_prob_stay_home(double);
  static void get_disease_parameters();
  void increment_S_count() { epidemic->increment_S_count();}
  void decrement_S_count() { epidemic->decrement_S_count();}
  void increment_E_count() { epidemic->increment_E_count();}
  void decrement_E_count() { epidemic->decrement_E_count();}
  void increment_I_count() { epidemic->increment_I_count();}
  void decrement_I_count() { epidemic->decrement_I_count();}
  void increment_i_count() { epidemic->increment_i_count();}
  void decrement_i_count() { epidemic->decrement_i_count();}
  void increment_R_count() { epidemic->increment_R_count();}
  void decrement_R_count() { epidemic->decrement_R_count();}
  void increment_r_count() { epidemic->increment_r_count();}
  void decrement_r_count() { epidemic->decrement_r_count();}
  void increment_C_count() { epidemic->increment_C_count();}
  void decrement_C_count() { epidemic->decrement_C_count();}
  void increment_c_count() { epidemic->increment_c_count();}
  void decrement_c_count() { epidemic->decrement_c_count();}
  void increment_M_count() { epidemic->increment_M_count();}
  void decrement_M_count() { epidemic->decrement_M_count();}

private:
  int id;
  double transmissibility;

  double immunity_loss_rate;
  double *mutation_prob;
  double mortality_rate;
  Epidemic *epidemic;
  Age_Map *residual_immunity;
  Age_Map *at_risk;
  StrainTable *strainTable;
  IntraHost *ihm;
  Evolution *evol;

  // Vars that are not Disease-specific (for updating global stats).
  Population *population;
};

inline void Disease::add_infectious_place(Place *p, char type) {
  epidemic->add_infectious_place(p, type);
}

inline void Disease::insert_into_infectious_list(Person * per) {
  epidemic->insert_into_infectious_list(per);
}

inline void Disease::insert_into_infected_list(Person * per) {
  epidemic->insert_into_infected_list(per);
}

inline void Disease::remove_from_infectious_list(Person * per) {
  epidemic->remove_from_infectious_list(per);
}

#endif // _FRED_Disease_H

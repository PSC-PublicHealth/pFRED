/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Disease.h
//

#ifndef _FRED_DISEASE_H
#define _FRED_DISEASE_H

#include "Global.h"
#include "Epidemic.h"

using namespace std;

class Person;
class Population;
class Age_Map;

class Disease {
public:
  Disease();
  UNIT_TEST_VIRTUAL ~Disease();
	
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void setup(int s, Population *pop,  double *mut_prob);
  UNIT_TEST_VIRTUAL void print();
  UNIT_TEST_VIRTUAL void update(int day);
	
  // These methods draw from the underlying distributions to randomly determine some aspect of the infection.
  UNIT_TEST_VIRTUAL int get_days_latent();
  UNIT_TEST_VIRTUAL int get_days_incubating();
  UNIT_TEST_VIRTUAL int get_days_asymp();
  UNIT_TEST_VIRTUAL int get_days_symp();
  UNIT_TEST_VIRTUAL int get_days_recovered();
  UNIT_TEST_VIRTUAL int get_symptoms();
  UNIT_TEST_VIRTUAL double get_asymp_infectivity() {return asymp_infectivity;}
  UNIT_TEST_VIRTUAL double get_symp_infectivity() {return symp_infectivity;}
  UNIT_TEST_VIRTUAL double get_mortality_rate() {return mortality_rate;}
  UNIT_TEST_VIRTUAL int get_max_days() { return max_days; }
	
  UNIT_TEST_VIRTUAL int get_id() { return id; }
  UNIT_TEST_VIRTUAL int get_infection_model() { return infection_model; }
  UNIT_TEST_VIRTUAL double get_transmissibility() { return transmissibility; }
  UNIT_TEST_VIRTUAL double get_prob_symptomatic() { return prob_symptomatic; }
  UNIT_TEST_VIRTUAL double get_attack_rate();
  UNIT_TEST_VIRTUAL Age_Map* get_residual_immunity() const { return residual_immunity; }
  UNIT_TEST_VIRTUAL Age_Map* get_at_risk() const { return at_risk; }
	
  // Draws from this disease's mutation probability distribution, and returns
  // the disease to mutate to, or NULL if no mutation should occur.
  Disease* should_mutate_to();
	
  UNIT_TEST_VIRTUAL void insert_into_infected_list(Person *person);
  UNIT_TEST_VIRTUAL void insert_into_infectious_list(Person *person);
  UNIT_TEST_VIRTUAL void remove_from_infectious_list(Person *person);
  UNIT_TEST_VIRTUAL void update_stats(int day);
  UNIT_TEST_VIRTUAL void print_stats(int day);
	
  UNIT_TEST_VIRTUAL Population * get_population() { return population; }
  UNIT_TEST_VIRTUAL Epidemic * get_epidemic() { return epidemic; }
	
  static double get_prob_stay_home();
  static void set_prob_stay_home(double);
  static void get_disease_parameters();
  void increment_S_count() { epidemic->increment_S_count(); }
  void decrement_S_count() { epidemic->decrement_S_count(); }
  void increment_E_count() { epidemic->increment_E_count(); }
  void decrement_E_count() { epidemic->decrement_E_count(); }
  void increment_I_count() { epidemic->increment_I_count(); }
  void decrement_I_count() { epidemic->decrement_I_count(); }
  void increment_i_count() { epidemic->increment_i_count(); }
  void decrement_i_count() { epidemic->decrement_i_count(); }
  void increment_R_count() { epidemic->increment_R_count(); }
  void decrement_R_count() { epidemic->decrement_R_count(); }
  void increment_r_count() { epidemic->increment_r_count(); }
  void decrement_r_count() { epidemic->decrement_r_count(); }
  void increment_C_count() { epidemic->increment_C_count(); }
  void decrement_C_count() { epidemic->decrement_C_count(); }
  void increment_c_count() { epidemic->increment_c_count(); }
  void decrement_c_count() { epidemic->decrement_c_count(); }
  void increment_M_count() { epidemic->increment_M_count(); }
  void decrement_M_count() { epidemic->decrement_M_count(); }
  
private:
  int id;
  double transmissibility;
  double prob_symptomatic;
  double asymp_infectivity;
  double symp_infectivity;
  int infection_model;
  int max_days_latent;
  int max_days_incubating;
  int max_days_asymp;
  int max_days_symp;
  int max_days;
  double *days_latent;
  double *days_incubating;
  double *days_asymp;
  double *days_symp;
  double immunity_loss_rate;
  double *mutation_prob;
  double mortality_rate;
  Epidemic *epidemic;
  Age_Map *residual_immunity;
  Age_Map *at_risk;
  
  // Vars that are not disease-specific (for updating global stats).
  Population *population;
};

inline void Disease::insert_into_infectious_list(Person * per) {
  epidemic->insert_into_infectious_list(per);
}

inline void Disease::insert_into_infected_list(Person * per) {
  epidemic->insert_into_infected_list(per);
}

inline void Disease::remove_from_infectious_list(Person * per) {
  epidemic->remove_from_infectious_list(per);
}

#endif // _FRED_DISEASE_H

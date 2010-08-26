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

#include "Global.h"
#include <set>
#include <stack>
using namespace std;

class Infection;
class Person;
class Place;
class Population;
class Spread;
class Age_Map;

class Strain {
public:
  Strain();
  UNIT_TEST_VIRTUAL ~Strain();
	
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void setup(int s, Population *pop,  double *mut_prob);
  UNIT_TEST_VIRTUAL void print();
  UNIT_TEST_VIRTUAL void update(int day);
	
  // If the infectee is susceptible, is at the given place on the given day,
  // and if the random roll for transmissibility succeeds
  // the infection is successful and attempt_infection returns true.
  UNIT_TEST_VIRTUAL bool attempt_infection(Person* infector, Person* infectee, Place* place, int exposure_date);
	
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
	
  // Draws from this strain's mutation probability distribution, and returns
  // the strain to mutate to, or NULL if no mutation should occur.
  Strain* should_mutate_to();
	
  UNIT_TEST_VIRTUAL void insert_into_exposed_list(Person *person);
  UNIT_TEST_VIRTUAL void insert_into_infectious_list(Person *person);
  UNIT_TEST_VIRTUAL void remove_from_exposed_list(Person *person);
  UNIT_TEST_VIRTUAL void remove_from_infectious_list(Person *person);
  UNIT_TEST_VIRTUAL void update_stats(int day);
  UNIT_TEST_VIRTUAL void print_stats(int day);
	
  UNIT_TEST_VIRTUAL Population * get_population() { return population; }
  UNIT_TEST_VIRTUAL Spread * get_spread() { return spread; }
	
  static double get_prob_stay_home();
  static void set_prob_stay_home(double);
  static void get_strain_parameters();
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
  Spread *spread;
  Age_Map *residual_immunity;
  Age_Map *at_risk;
  
  // Vars that are not strain-specific (for updating global stats).
  Population *population;
};


#endif // _FRED_STRAIN_H

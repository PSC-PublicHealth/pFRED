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

  /**
   * Default constructor
   */
  Disease();
  ~Disease();

  /**
   * Set all of the attributes for the Disease
   *
   * @param s the ID of the Disease
   * @param pop the population for this Disease
   * @param mut_prob the mutation probability of this disease
   */
  void setup(int s, Population *pop,  double *mut_prob);

  /**
   * Print out information about this object
   */
  void print();

  /**
   * Perform the daily update for this object
   *
   * @param day the simulation day
   */
  void update(int day);

  // The methods draw from the underlying distributions to randomly determine some aspect of the infection
  // have been moved to the DefaultIntraHost class

  /**
   * @return the intrahost model's days symptomatic
   * @see IntraHost::get_days_symp()
   */
  int get_days_symp();

  /**
   * @return the days recovered
   */
  int get_days_recovered();

  /**
   * @return the intrahost model's max_days
   * @see IntraHost::get_max_days()
   */
  int get_max_days();

  /**
   * @return this Disease's mortality_rate
   */
  double get_mortality_rate() { return mortality_rate;}

  /**
   * @return this Disease's id
   */
  int get_id() { return id;}

  /**
   * @return the transmissibility
   */
  double get_transmissibility() { return transmissibility;}

  /**
   * @param strain the strain of the disease
   * @return the strain's transmissibility
   */
  double get_transmissibility(int strain);

  /**
   * @return the Epidemic's attack rate
   * @see Epidemic::get_attack_rate()
   */
  double get_attack_rate();

  /**
   * @return a pointer to this Disease's residual_immunity Age_Map
   */
  Age_Map * get_residual_immunity() const { return residual_immunity;}

  /**
   * @return a pointer to this Disease's at_risk Age_Map
   */
  Age_Map * get_at_risk() const { return at_risk;}

  /**
   * @param the simulation day
   * @return a pointer to a map of Primary Loads for a given day
   * @see Evolution::getPrimaryLoads(int day)
   */
  map<int, double> * getPrimaryLoads(int day);

  /**
   * @param the simulation day
   * @param the particular strain of the disease
   * @return a pointer to a map of Primary Loads for a particular strain of the disease on a given day
   * @see Evolution::getPrimaryLoads(int day, int strain)
   */
  map<int, double> * getPrimaryLoads(int day, int strain);

  /**
   * @return a pointer to this Disease's Evolution attribute
   */
  Evolution * get_evolution() { return evol;}

  /**
   * @param infection
   * @param loads
   * @return a pointer to a Trajectory object
   * @see return Trajectory::getTrajectory(Infection *infection, map<int, double> *loads)
   */
  Trajectory * getTrajectory(Infection *infection, map<int, double> *loads);

  /**
   * Draws from this Disease's mutation probability distribution
   * @returns the Disease to mutate to, or NULL if no mutation should occur.
   */
  Disease * should_mutate_to();

  /**
   * Add a person to the Epidemic's infectious place list
   * @param p pointer to a Place
   * @param type the type of Place
   * @see Epidemic::add_infectious_place(Place *p, char type)
   */
  void add_infectious_place(Place *p, char type);

  /**
   * Add a person to the Epidemic's infected list
   * @param per a pointer to a person
   * @see Epidemic::insert_into_infected_list(Person *person)
   */
  void insert_into_infected_list(Person *person);

  /**
   * Add a person to the Epidemic's infectious list
   * @param per a pointer to a Person
   * @see Epidemic::insert_into_infectious_list(Person *person)
   */
  void insert_into_infectious_list(Person *person);

  /**
   * Remove a person to the Epidemic's infectious list
   * @param per a pointer to a Person
   * @see Epidemic::remove_from_infectious_list(Person *person)
   */
  void remove_from_infectious_list(Person *person);

  /**
   * @param day the simulation day
   * @see Epidemic::update_stats(int day);
   */
  void update_stats(int day);

  /**
   * @param day the simulation day
   * @see Epidemic::print_stats(day);
   */
  void print_stats(int day);

  /**
   * @return the population with which this Disease is associated
   */
  Population * get_population() { return population;}

  /**
    * @return the epidemic with which this Disease is associated
    */
  Epidemic * get_epidemic() { return epidemic;}

  /**
   * @return the probability that agent's will stay home
   */
  static double get_prob_stay_home();

  /**
   * @param the new probability that agent's will stay home
   */
  static void set_prob_stay_home(double);


  static void get_disease_parameters();

  /**
   * Increment the Susceptible count
   * @see Epidemic::increment_S_count()
   */
  void increment_S_count() { epidemic->increment_S_count();}

  /**
   * Decrement the Susceptible
   * @see Epidemic::decrement_S_count()
   */
  void decrement_S_count() { epidemic->decrement_S_count();}

  /**
   * Increment the Exposed count
   * @see Epidemic::increment_E_count()
   */
  void increment_E_count() { epidemic->increment_E_count();}

  /**
   * Decrement the Exposed
   * @see Epidemic::decrement_E_count()
   */
  void decrement_E_count() { epidemic->decrement_E_count();}

  /**
   * Increment the Infectious count
   * @see Epidemic::increment_I_count()
   */
  void increment_I_count() { epidemic->increment_I_count();}

  /**
   * Decrement the Infectious
   * @see Epidemic::decrement_I_count()
   */
  void decrement_I_count() { epidemic->decrement_I_count();}

  /**
   * Increment the i_count
   * @see Epidemic::increment_i_count()
   */
  void increment_i_count() { epidemic->increment_i_count();}

  /**
   * Decrement the i_count
   * @see Epidemic::decrement_i_count()
   */
  void decrement_i_count() { epidemic->decrement_i_count();}

  /**
   * Increment the Recovered count
   * @see Epidemic::increment_R_count()
   */
  void increment_R_count() { epidemic->increment_R_count();}

  /**
   * Decrement the Recovered count
   * @see Epidemic::decrement_R_count()
   */
  void decrement_R_count() { epidemic->decrement_R_count();}

  /**
   * Increment the r_count
   * @see Epidemic::increment_r_count()
   */
  void increment_r_count() { epidemic->increment_r_count();}

  /**
   * Decrement the r_count
   * @see Epidemic::decrement_r_count()
   */
  void decrement_r_count() { epidemic->decrement_r_count();}

  /**
   * Increment the C_count
   * @see Epidemic::increment_C_count()
   */
  void increment_C_count() { epidemic->increment_C_count();}

  /**
   * Decrement the C_count
   * @see Epidemic::decrement_C_count()
   */
  void decrement_C_count() { epidemic->decrement_C_count();}

  /**
   * Increment the c_count
   * @see Epidemic::increment_c_count()
   */
  void increment_c_count() { epidemic->increment_c_count();}

  /**
   * Decrement the c_count
   * @see Epidemic::decrement_c_count()
   */
  void decrement_c_count() { epidemic->decrement_c_count();}

  /**
   * Increment the Immune count
   * @see Epidemic::increment_M_count()
   */
  void increment_M_count() { epidemic->increment_M_count();}

  /**
   * Decrement the Immune count
   * @see Epidemic::decrement_M_count()
   */
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

  static double Prob_stay_home;

  // Vars that are not Disease-specific (for updating global stats).
  Population *population;
};

inline void Disease::add_infectious_place(Place *p, char type) {
  epidemic->add_infectious_place(p, type);
}

inline void Disease::insert_into_infected_list(Person *per) {
  epidemic->insert_into_infected_list(per);
}

inline void Disease::insert_into_infectious_list(Person *per) {
  epidemic->insert_into_infectious_list(per);
}

inline void Disease::remove_from_infectious_list(Person *per) {
  epidemic->remove_from_infectious_list(per);
}

#endif // _FRED_Disease_H

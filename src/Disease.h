/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Disease.h
//

#ifndef _FRED_Disease_H
#define _FRED_Disease_H

#include <map>
#include <fstream>

#include "Global.h"
#include "Epidemic.h"
#include "Transmission.h"

using namespace std;

class Person;
class Population;
class Age_Map;
class StrainTable;
class Strain;
class IntraHost;
class Trajectory;
class Infection;
class Strain_Data;

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
  double get_transmissibility( int strain );

  /**
   * @param seasonality_value meterological condition (eg specific humidity in kg/kg)
   * @return the multiplier calculated from the seasonality condition; attenuates transmissibility
   */
  double calculate_climate_multiplier( double seasonality_value );

  /**
   * @return the Epidemic's attack rate
   * @see Epidemic::get_attack_rate()
   */
  double get_attack_rate();

  /**
   * @return the Epidemic's clincial attack rate
   * @see Epidemic::get_attack_rate()
   */
  double get_clinical_attack_rate();

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
   * @see Evolution::get_primary_loads(int day)
   */
  Transmission::Loads * get_primary_loads(int day);

  /**
   * @param the simulation day
   * @param the particular strain of the disease
   * @return a pointer to a map of Primary Loads for a particular strain of the disease on a given day
   * @see Evolution::get_primary_loads(int day, int strain)
   */
  Transmission::Loads * get_primary_loads(int day, int strain);

  /**
   * @return a pointer to this Disease's Evolution attribute
   */
  Evolution * get_evolution() { return evol;}

  /**
   * @param infection
   * @param loads
   * @return a pointer to a Trajectory object
   * @see return Trajectory::get_trajectory(Infection *infection, map<int, double> *loads)
   */
  Trajectory * get_trajectory( Infection *infection, Transmission::Loads * loads );

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
  void add_infectious_place(Place *p, char type) {
    epidemic->add_infectious_place(p, type);
  }

  /**
   * @param day the simulation day
   * @see Epidemic::print_stats(day);
   */
  void print_stats(int day) { epidemic->print_stats(day); }

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
  static void set_prob_stay_home( double );

  int get_num_strains();
  int get_num_strain_data_elements( int strain );
  int get_strain_data_element( int strain, int i );
  const Strain_Data & get_strain_data( int strain );
  const Strain & get_strain( int strain_id );

  static void get_disease_parameters();

  void become_susceptible( Person * person ) { epidemic->become_susceptible( person ); }
  void become_unsusceptible( Person * person ) { epidemic->become_unsusceptible( person ); }
  void become_exposed( Person * person ) { epidemic->become_exposed( person ); }
  void become_infectious( Person * person ) { epidemic->become_infectious( person ); }
  void become_uninfectious( Person * person ) { epidemic->become_uninfectious( person ); }
  void become_symptomatic( Person * person ) { epidemic->become_symptomatic( person ); }
  void become_removed( Person * person, bool susceptible, bool infectious, bool symptomatic ) {
    epidemic->become_removed( person, susceptible, infectious, symptomatic );
  }
  void become_immune( Person * person, bool susceptible, bool infectious, bool symptomatic ) {
    epidemic->become_immune( person, susceptible, infectious, symptomatic );
  }

  void increment_infectee_count( int day ) { epidemic->increment_infectee_count( day ); }
 
  bool gen_immunity_infection( int age );
  bool gen_immunity_vaccination( int age );

  int add_strain( Strain * child_strain, double transmissibility, int parent_strain_id );
  void add_root_strain( int num_elements );
  void printStrain( int strain_id, stringstream &out );
  std::string get_strain_data_string( int strain_id );
  void initialize_evolution_reporting_grid( Large_Grid * grid );
  
  double get_infectivity_threshold() { return infectivity_threshold; }
  double get_symptomaticity_threshold() {return symptomaticity_threshold; }

private:
  int id;
  double transmissibility;

  // infectivity and symptomaticity thresholds used in Infection class to
  // determine if an agent is infectious/symptomatic at a given point in the
  // trajectory
  double infectivity_threshold;
  double symptomaticity_threshold;

  double seasonality_max, seasonality_min;
  double seasonality_Ka, seasonality_Kb;

  double immunity_loss_rate;
  double *mutation_prob;
  double mortality_rate;
  Epidemic *epidemic;
  Age_Map *residual_immunity;
  Age_Map *at_risk;
  Age_Map *infection_immunity_prob;
  Age_Map *vaccination_immunity_prob;
  StrainTable *strain_table;
  IntraHost *ihm;
  Evolution *evol;

  static double Prob_stay_home;
  static double R0;
  static double R0_a;
  static double R0_b;

  // Vars that are not Disease-specific (for updating global stats).
  Population *population;
};

#endif // _FRED_Disease_H

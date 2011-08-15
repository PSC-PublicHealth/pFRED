/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Epidemic.h
//

#ifndef _FRED_EPIDEMIC_H
#define _FRED_EPIDEMIC_H

#include "Global.h"
#include <set>
#include <map>
#include <vector>

using namespace std;

class Disease;
class Person;
class Timestep_Map;
class Multistrain_Timestep_Map;
class Place;

class Epidemic {
public:
  Epidemic(Disease * str, Timestep_Map *);
  ~Epidemic();
  
  /**
   * Reset the epidemic
   */
  void clear();

  /**
   * Update the daily stats for the Epidemic
   * @param day the simulation day
   */
  void update_stats(int day);

  /**
   * Output daily Epidemic statistics to the files
   * @param day the simulation day
   */
  void print_stats(int day);

  /**
   * Add an infectious place of a given type to this Epidemic's list
   * @param p the Place to be added
   * @param type what type of place (H)ousehold, (O)ffice, etc
   */
  void add_infectious_place(Place *p, char type);

  /**
   *
   */
  void get_infectious_places(int day);

  /**
   * @param person the Person to be added to the infected list
   */
  void insert_into_infected_list(Person * person) { infected.push_back(person); }

  /**
   * @param person the Person to be added to the infecious list
   */
  void insert_into_infectious_list(Person * person) { infectious.insert(person); }

  /**
   * @param person the Person to be removed from the infecious list
   */
  void remove_from_infectious_list(Person * person) { infectious.erase(person); }

  /**
   * @return the attack rate
   */
  double get_attack_rate() { return attack_rate; }


  //void update(Date *sim_start_date, int day);
  void update(int day);

  /**
   * @return the clinical_incidents
   */
  int get_clinical_incidents() { return clinical_incidents; }

  /**
   * @return the incident_infections
   */
  int get_incident_infections() { return incident_infections; }

  /**
   * @return the total_incidents
   */
  int get_total_incidents() { return total_incidents; }

  /**
   * Increment the Susceptible count
   */
  void increment_S_count() { S_count++; }

  /**
   * Decrement the Susceptible count
   */
  void decrement_S_count() { S_count--; }

  /**
   * Increment the Exposed count
   */
  void increment_E_count() { E_count++; }

  /**
   * Decrement the Exposed count
   */
  void decrement_E_count() { E_count--; }

  /**
   * Increment the Infectious count
   */
  void increment_I_count() { I_count++; }

  /**
   * Decrement the Infectious count
   */
  void decrement_I_count() { I_count--; }

  /**
   * Increment the i_count
   */
  void increment_i_count() { i_count++; }

  /**
   * Decrement the i_count
   */
  void decrement_i_count() { i_count--; }

  /**
   * Increment the Recovered count
   */
  void increment_R_count() { R_count++; }

  /**
   * Decrement the Recovered count
   */
  void decrement_R_count() { R_count--; }

  /**
    * Increment the r_count
    */
  void increment_r_count() { r_count++; }

  /**
   * Decrement the r_count
   */
  void decrement_r_count() { r_count--; }

  /**
   * Increment the C_count count
   */
  void increment_C_count() { C_count++; }

  /**
   * Decrement the C_count count
   */
  void decrement_C_count() { C_count--; }

  /**
   * Increment the c_count count
   */
  void increment_c_count() { c_count++; }

  /**
   * Decrement the c_count count
   */
  void decrement_c_count() { c_count--; }

  /**
   * Increment the Immune count
   */
  void increment_M_count() { M_count++; }

  /**
   * Decrement the Immune count
   */
  void decrement_M_count() { M_count--; }
  
private:
  Disease * disease;
  int id;
  int N;				      // current population size
  int N_init;				      // initial population size
  vector <Place *> inf_households;
  vector <Place *> inf_neighborhoods;
  vector <Place *> inf_classrooms;
  vector <Place *> inf_schools;
  vector <Place *> inf_workplaces;
  vector <Place *> inf_offices;
  double attack_rate;
  double clinical_attack_rate;
  set <Person *> infectious;
  Timestep_Map* primary_cases_map;
  int clinical_incidents;
  int total_clinical_incidents;
  int incident_infections;
  int total_incidents;
  int vaccine_acceptance;
  int * new_cases;
  vector <Person *> infected;
  int r_index;
  double RR;
  int NR;
  int S_count;
  int E_count;
  int I_count;
  int i_count;
  int R_count;
  int r_count;
  int C_count;
  int c_count;
  int M_count;
  map<int, int> incidence;
  map<int, int> prevalence;
};

#endif // _FRED_EPIDEMIC_H

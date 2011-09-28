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
   * @return the attack rate
   */
  double get_attack_rate() { return attack_rate; }

  void get_primary_infections(int day);
  void transmit(int day);

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

  void become_susceptible(Person *person);
  void become_unsusceptible(Person *person);
  void become_exposed(Person *person);
  void become_infectious(Person *person, bool symptomatic);
  void become_uninfectious(Person *person);
  void become_symptomatic(Person *person);
  void become_removed(Person *person, bool susceptible, bool infectious, bool symptomatic);
  void become_immune(Person *person, bool susceptible, bool infectious, bool symptomatic);

  void find_infectious_places(int day, int dis);
  void add_susceptibles_to_infectious_places(int day, int dis);

  // static methods
  static void update(int day);
  static void transmit_infection(int day);
  static void get_visitors_to_infectious_places(int day);

private:
  Disease * disease;
  int id;
  int N;				      // current population size
  int N_init;				      // initial population size
  typedef pair<Person *,int> person_pair;
  struct person_pair_comparator {
    bool operator()(const person_pair A, const person_pair B) const  {
      return A.second < B.second;
    }
  };

  set <person_pair, person_pair_comparator> susceptible_list;
  set <person_pair, person_pair_comparator> infectious_list;
  vector <Person *> exposed_list;
  vector <Place *> inf_households;
  vector <Place *> inf_neighborhoods;
  vector <Place *> inf_classrooms;
  vector <Place *> inf_schools;
  vector <Place *> inf_workplaces;
  vector <Place *> inf_offices;
  double attack_rate;
  double clinical_attack_rate;
  Timestep_Map* primary_cases_map;
  int clinical_incidents;
  int total_clinical_incidents;
  int incident_infections;
  int total_incidents;
  int vaccine_acceptance;
  int * new_cases;
  int rr_index;
  double RR;
  int NR;
  int E_count;
  int Symp_count;
  int R_count;
  int C_count;
  int c_count;
  int M_count;
  map<int, int> incidence;
  map<int, int> prevalence;
};

#endif // _FRED_EPIDEMIC_H

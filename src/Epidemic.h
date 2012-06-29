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

  void report_age_of_infection(int day);
  void report_place_of_infection(int day);
  void report_presenteeism(int day);

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
   * @return the clinical_incidents
   */
  int get_total_clinical_incidents() { return total_clinical_incidents; }

  /**
   * @return the clinical_attack_rate
   */
  double get_clinical_attack_rate() { return clinical_attack_rate; }

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
  void become_infectious(Person *person);
  void become_uninfectious(Person *person);
  void become_symptomatic(Person *person);
  void become_removed(Person *person, bool susceptible, bool infectious, bool symptomatic);
  void become_immune(Person *person, bool susceptible, bool infectious, bool symptomatic);

  void find_infectious_places(int day, int dis);
  void add_susceptibles_to_infectious_places(int day, int dis);
  void increment_infectee_count(int day) {
    #pragma omp atomic
    ++( infectees[ day ] );
  }

  int get_num_infectious(); 
  void get_infectious_samples(int num_samples, vector<Person *> &samples);
  void get_infectious_samples(vector<Person *> &samples, double prob);

  // static methods
  static void update(int day);
  static void transmit_infection(int day);
  static void get_visitors_to_infectious_places(int day);

private:
  Disease * disease;
  int id;
  int N;          // current population size
  int N_init;     // initial population size
  
  Timestep_Map* primary_cases_map;
  // lists of susceptible and infectious Persons now kept as
  // bit maskes in Population
  // set <person_pair, person_pair_comparator> susceptible_list;
  // set <person_pair, person_pair_comparator> infectious_list;
  vector <Person *> daily_infections_list;
  vector <Place *> inf_households;
  vector <Place *> inf_neighborhoods;
  vector <Place *> inf_classrooms;
  vector <Place *> inf_schools;
  vector <Place *> inf_workplaces;
  vector <Place *> inf_offices;
  double attack_rate;
  double clinical_attack_rate;
  int clinical_incidents;
  int total_clinical_incidents;
  int incident_infections;
  int total_incidents;
  int * new_cases;
  int * infectees;
  double RR;    // reproductive rate
  int cohort_size;
  int exposed_count;
  int symptomatic_count;
  int removed_count;
  int immune_count;

  struct update_susceptible_activities {
    int day, disease_id;
    update_susceptible_activities( int _day, int _disease_id ) : day( _day ), disease_id( _disease_id ) { };
    void operator() ( Person & p );
  };

  struct update_infectious_activities {
    int day, disease_id;
    update_infectious_activities( int _day, int _disease_id ) : day( _day ), disease_id( _disease_id ) { };
    void operator() ( Person & p );
  };

  struct infectious_sampler {
    double prob;
    vector< Person * > * samples;
    void operator() ( Person & p );
  };

};

#endif // _FRED_EPIDEMIC_H

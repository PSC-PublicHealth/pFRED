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
// File: Epidemic.h
//

#ifndef _FRED_EPIDEMIC_H
#define _FRED_EPIDEMIC_H

#include <set>
#include <map>
#include <vector>

#include "Global.h"
#include "Place.h"

#define SEED_USER 'U'
#define SEED_RANDOM 'R'
#define SEED_EXPOSED 'E'
#define SEED_INFECTIOUS 'I'

using namespace std;

class Disease;
class Person;
class Timestep_Map;
class Multistrain_Timestep_Map;
//class Place;

class Epidemic {
public:
  Epidemic(Disease * str, Timestep_Map *);
  ~Epidemic();
 
  void setup();

  /**
   * Output daily Epidemic statistics to the files
   * @param day the simulation day
   */
  void print_stats(int day);

  void report_age_of_infection(int day);
  void report_transmission_by_age_group(int day);
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

  void get_primary_infections(int day);

  void transmit(int day);

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

  void increment_cohort_infectee_count(int cohort_day) {
    #pragma omp atomic
    ++( number_infected_by_cohort[ cohort_day ] );
  }

  void get_infectious_samples(int num_samples, vector<Person *> &samples);
  void get_infectious_samples(vector<Person *> &samples, double prob);

  int get_susceptible_people() { return susceptible_people; }
  int get_exposed_people() { return exposed_people; }
  int get_infectious_people() { return infectious_people; }
  int get_removed_people() { return removed_people; }
  int get_immune_people() { return immune_people; }
  int get_people_becoming_infected_today() { return people_becoming_infected_today; }
  int get_total_people_ever_infected() { return total_people_ever_infected; }
  int get_people_becoming_symptomatic_today() { return people_becoming_symptomatic_today; }
  int get_people_with_current_symptoms() { return people_with_current_symptoms; }
  double get_RR() { return RR; }
  double get_attack_ratio() { return attack_ratio; }
  double get_symptomatic_attack_ratio() { return symptomatic_attack_ratio; }

  int get_incidence() { return incidence; }
  int get_symptomatic_incidence () { return symptomatic_incidence; }
  int get_prevalence_count () { return prevalence_count; }
  double get_prevalence () { return prevalence; }
  int get_incident_infections() { return get_incidence(); }

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

  // valid seeding types are:
  // "user_specified" => SEED_USER 'U'
  // "random" => SEED_RANDOM 'R'
  // see Epidemic::advance_seed_infection"
  char seeding_type_name[ FRED_STRING_SIZE ];
  char seeding_type;
  double fraction_seeds_infectious; 

  /// advances infection either to the first infetious day (SEED_INFECTIOUS)
  /// or to a random day in the trajectory (SEED_RANDOM)
  /// this is accomplished by moving the exposure date back as appropriate;
  /// (ultimately done in Infection::advance_infection)
  void advance_seed_infection( Person * person );

  // lists of susceptible and infectious Persons now kept as
  // bit maskes in Population "Bloque"
  vector <Person *> daily_infections_list;

  vector <Place *> inf_households;
  vector <Place *> inf_neighborhoods;
  vector <Place *> inf_classrooms;
  vector <Place *> inf_schools;
  vector <Place *> inf_workplaces;
  vector <Place *> inf_offices;

  // population health state counters
  int susceptible_people;
  int exposed_people;
  int infectious_people;
  int removed_people;
  int immune_people;

  int people_becoming_infected_today;
  int total_people_ever_infected;

  int people_becoming_symptomatic_today;
  int people_with_current_symptoms;
  int total_people_ever_symptomatic;

  // used for computing reproductive rate:
  double RR;
  int * daily_cohort_size;
  int * number_infected_by_cohort;

  // attack ratios
  double attack_ratio;
  double symptomatic_attack_ratio;

  // used for maintining quantities from previous day;
  int incidence;
  int symptomatic_incidence;
  int prevalence_count;
  double prevalence;

  //fred::Mutex mutex;
  fred::Spin_Mutex neighborhood_mutex;
  fred::Spin_Mutex household_mutex;
  fred::Spin_Mutex workplace_mutex;
  fred::Spin_Mutex office_mutex;
  fred::Spin_Mutex school_mutex;
  fred::Spin_Mutex classroom_mutex;

  fred::Spin_Mutex spin_mutex;

  size_t place_person_list_reserve_size;

  // /////////// Functors for Population loops //////////////////

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

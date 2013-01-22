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
// File: Population.h
//

#ifndef _FRED_POPULATION_H
#define _FRED_POPULATION_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "Global.h"
#include "Demographics.h"
#include "Bloque.h"
#include "Compression.h"

class Person;
class Disease;
class Antivirals;
class AV_Manager;
class Vaccine_Manager;
class Place;

using namespace std;

typedef map <Person*, bool> ChangeMap;  

class Population {
public:

  /**
   * Default constructor
   */
  Population();
  ~Population();

  void initialize_masks();

  /**
   * Sets the static variables for the class from the parameter file.
   */
  void get_parameters();

  /**
   * Prepare this Population (calls read_population)
   * @see Population::read_population()
   */
  void setup();

  /**
   * Used during debugging to verify that code is functioning properly.
   */
  void quality_control();

  /**
   * Print out information about this object.  This print can be adjusted to limit the output to just those
   * agents who have changed status (0:print all, 1:incremental, -1:unchanged) with the incremental flag.
   * @param incremental the flag to adjust the agents who are actually printed out
   * @param day the simulation day
   */
  void print(int incremental=0, int day=0);

  /**
   * Perform end of run operations (clean up)
   */
  void end_of_run();

  /**
   * Perform beginning of day operations
   * @param day the simulation day
   */
  void update(int day);

  /**
   * Report the disease statistics for a given day
   * @param day the simulation day
   */
  void report(int day);

  /**
   * @param disease_id the index of the Disease
   * @return a pointer to the Disease indexed by s
   */
  Disease * get_disease(int disease_id);

  /**
   * @return the pop_size
   */
  int get_pop_size() { return pop_size; }

  //Mitigation Managers
  /**
   * @return a pointer to this Population's AV_Manager
   */
  AV_Manager * get_av_manager(){ return av_manager; }

  /**
   * @return a pointer to this Population's Vaccine_Manager
   */
  Vaccine_Manager * get_vaccine_manager() { return vacc_manager;}

  /**
   * @param args passes to Person ctor; all persons added to the
   * Population must be created through this method
   *
   * @return pointer to the person created and added
   */
  Person * add_person( int age, char sex, int race, int rel, Place *house,
      Place *school, Place *work, int day, bool today_is_birthday );

  /**
   * @param per a pointer to the Person to remove from the Population
   */
  void delete_person(Person * per);

  /**
   * Perform the necessary steps for an agent's death
   * @param day the simulation day
   * @param person_index the population index of the agent who will die
   */
  void prepare_to_die( int day, int person_index );

  /**
   * Perform the necessary steps for an agent to give birth
   * @param day the simulation day
   * @param person_index the population index of the agent who will give birth
   */
  void prepare_to_give_birth( int day, int person_index );
  
  /**
   * @param index the index of the Person
   * Return a pointer to the Person object at this index
   */
  Person * get_person_by_index( int index );

  /**
   * @param n the id of the Person
   * Return a pointer to the Person object with this id
   */
  Person * get_person_by_id( int id );

  // Modifiers on the entire pop;
  // void apply_residual_immunity(Disease *disease) {}

  // track those agents that have changed since the last incremental dump
  /**
   * Sets the changed flag for a Person so that the incremental print option will know that the agent has indeed changed
   * @param p pointer to the Person object that has changed
   */
  void set_changed(Person *p);

  /**
   * Assign agents in Schools to specific Classrooms within the school
   */
  void assign_classrooms();
  
  /**
   * Assign agents in Workplaces to specific Offices within the workplace
   */
  void assign_offices();

  /**
   * Write degree information to a file degree.txt
   * @param directory the directory where the file will be written
   */
  void get_network_stats(char *directory);

  /**
   * Read the population from the population file
   */
  void read_population();

  /**
   * Print the birth information to the status file
   * @see Global::Birthfp
   * @param day the simulation day
   * @param per a pointer to the Person object that has given birth
   */
  void report_birth(int day, Person *per) const;

  /**
   * Print the death information to the status file
   * @see Global::Deathfp
   * @param day the simulation day
   * @param per a pointer to the Person object that has died
   */
  void report_death(int day, Person *per) const;

  /**
   * @param id the id of the Person
   * @return the population string used during setup that matches the id
   */
  char * get_pstring(int id);

  /**
   *
   */
  void print_age_distribution(char *dir, char * date_string, int run);

  /**
   * @return a pointer to a random Person in this population
   */
  Person * select_random_person();

  /**
   * @return a pointer to a random Person in this population
   * whose age is in the given range
   */
  Person * select_random_person_by_age(int min_age, int max_age);

  /*
   * Set the mask bit for the person_index
   *
   * TODO redefine the mask type so that multiple sets of masks
   * are available (one set for each disease)
   */
  void set_mask_by_index( fred::Population_Masks mask, int person_index );

  /*
   * Clear the mask bit for the person_index
   *
   * TODO redefine the mask type so that multiple sets of masks
   * are available (one set for each disease)
   */
  void clear_mask_by_index( fred::Population_Masks mask, int person_index );

  /*
   * Check to see if mask is set for person_index
   */
  bool check_mask_by_index( fred::Population_Masks mask, int person_index ) {
    return blq.mask_is_set( mask, person_index );
  }

  int size() {
    assert( blq.size() == pop_size );
    return blq.size();
  }

  int size( fred::Population_Masks mask ) { return blq.size( mask ); }

  template< typename Functor >
  void apply( Functor & f ) { blq.apply( f ); }

  template< typename Functor >
  void apply( fred::Population_Masks m, Functor & f ) { blq.apply( m, f ); }

  template< typename Functor >
  void parallel_apply( Functor & f ) { blq.parallel_apply( f ); }

  template< typename Functor >
  void parallel_masked_apply( fred::Population_Masks m, Functor & f ) { blq.parallel_masked_apply( m, f ); }

  template< typename Functor >
  void parallel_not_masked_apply( fred::Population_Masks m, Functor & f ) { blq.parallel_not_masked_apply( m, f ); }

  template< typename Functor >
  void parallel_apply_with_thread_id( fred::Population_Masks m, Functor & f ) {
    blq.parallel_masked_apply_with_thread_id( m, f );
  }


  /* TODO rewrite
  template< typename MaskType >
  struct masked_iterator : bloque< Person, fred::Population_Masks >::masked_iterator< MaskType > { };

  template< typename MaskType >
  masked_iterator< MaskType > begin() { return blq.begin(); }

  template< typename MaskType >
  masked_iterator< MaskType > end() { return blq.end(); }
  */

private:

  struct Person_Init_Data {
    int age, race, relationship;
    char sex;
    bool today_is_birthday;
    int day;
    Place * house;
    Place * work;
    Place * school;
    
    Person_Init_Data( int _age, int _race, int _relationship,
        char _sex, bool _today_is_birthday, int _day ) {

      age = _age;
      race = _race;
      relationship = _relationship;
      sex = _sex;
      today_is_birthday = _today_is_birthday;
      day = _day;
    }
  };

  void parse_lines_from_stream( std::istream & stream );

  bloque< Person, fred::Population_Masks > blq;   // all Persons in the population
  //std::map< int, int > id_to_index;
  vector <Person * > death_list;     // list agents to die today
  vector <Person * > maternity_list; // list agents to give birth today
  int pop_size;
  Disease *disease;

  vector <Person *> birthday_vecs[367]; //0 won't be used | day 1 - 366
  map<Person *, int > birthday_map;

  double **mutation_prob;
  ChangeMap incremental_changes;  // incremental "list" (actually a C++ map)
                                  // of those agents whose stats
                                  // have changed since the last history dump
  ChangeMap never_changed;        // agents who have *never* changed

  static char profilefile[FRED_STRING_SIZE];
  static char pop_outfile[FRED_STRING_SIZE];
  static char output_population_date_match[FRED_STRING_SIZE];
  static int output_population;
  static bool is_initialized;
  static int next_id;

  vector<int> dead;

  //Mitigation Managers
  AV_Manager *av_manager;
  Vaccine_Manager *vacc_manager;

  /**
   * Used for reporting
   */
  void clear_static_arrays();

  /**
   * Write out the population in a format similar to the population input files (with additional runtime information)
   * @param day the simulation day
   */
  void write_population_output_file(int day);
  
  // functors for demographics updates 
  struct update_population_births {
    int day;
    update_population_births( int _day ) : day( _day ) { }
    void operator() ( Person & p );
  };
  struct update_population_deaths {
    int day;
    update_population_deaths( int _day ) : day( _day ) { }
    void operator() ( Person & p );
  };

  // functor for health update
  struct update_population_health {
    int day;
    update_population_health( int d ) : day( d ) { };
    void operator() ( Person & p );
  };

  fred::Mutex mutex;
  fred::Mutex add_person_mutex;
  fred::Mutex batch_add_person_mutex;

};

#endif // _FRED_POPULATION_H

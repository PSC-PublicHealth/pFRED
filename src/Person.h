/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Person.h
//

#ifndef _FRED_PERSON_H
#define _FRED_PERSON_H

#include "Global.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "Population.h"
#include "Demographics.h"
#include "Health.h"
#include "Behavior.h"
#include "Activities.h"
#include "Infection.h"
#include "Person_Event_Interface.h"
class Place;
class Disease;
class Date;

class Population;
class Transmission;

class Person {
public:
  Person();
  UNIT_TEST_VIRTUAL ~Person();
  UNIT_TEST_VIRTUAL void setup(int index, int age, char sex, int marital, int profession,
                               Place **favorite_places, Population* pop,
			       Date *sim_start_date, bool has_random_birthday);
  void setup(int index, int age, char sex, int marital, int occ,
	     char *house, char *school, char *work, Population *Pop,
	     Date *sim_start_date, bool has_random_birthday);
  UNIT_TEST_VIRTUAL void reset(Date *sim_start_date);
  UNIT_TEST_VIRTUAL void become_unsusceptible(int disease);
  UNIT_TEST_VIRTUAL void become_immune(Disease *disease);
  UNIT_TEST_VIRTUAL void update_infectious_activities(Date *sim_start_date, int day) {
    activities->update_infectious_activities(sim_start_date, day);
  }
  UNIT_TEST_VIRTUAL void update_susceptible_activities(Date *sim_start_date, int day) {
    activities->update_susceptible_activities(sim_start_date, day);
  }
  UNIT_TEST_VIRTUAL void print(FILE *fp, int disease) const;
  int is_symptomatic() { return health->is_symptomatic(); }
  bool is_susceptible(int dis) { return health->is_susceptible(dis); }
  bool is_infectious(int dis) { return health->is_infectious(dis); }
  int get_diseases();
  
  UNIT_TEST_VIRTUAL Place *get_household() const;
  UNIT_TEST_VIRTUAL Place *get_neighborhood() const;
  int get_age() const { return demographics->get_age(); }
  int get_init_age() const { return demographics->get_init_age(); }
  int get_init_profession() const { return demographics->get_init_profession(); }
  int get_init_marital_status() const { return demographics->get_init_marital_status(); }
  double get_real_age(int day) const { return demographics->get_real_age(day); }
  UNIT_TEST_VIRTUAL char get_sex() const;
  UNIT_TEST_VIRTUAL char get_marital_status() const;
  UNIT_TEST_VIRTUAL int get_profession() const;
  UNIT_TEST_VIRTUAL char get_disease_status(int disease) const { return health->get_disease_status(disease); }
  UNIT_TEST_VIRTUAL double get_susceptibility(int disease) const { return health->get_susceptibility(disease); }
  UNIT_TEST_VIRTUAL double get_infectivity(int disease, int day) const { return health->get_infectivity(disease, day); }
  UNIT_TEST_VIRTUAL int get_exposure_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_infectious_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_recovered_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_infector(int disease) const;
  UNIT_TEST_VIRTUAL int get_infected_place(int disease) const;
  UNIT_TEST_VIRTUAL char * get_infected_place_label(int disease) const;
  UNIT_TEST_VIRTUAL char get_infected_place_type(int disease) const;
  UNIT_TEST_VIRTUAL int get_infectees(int disease) const;
  UNIT_TEST_VIRTUAL int add_infectee(int disease);
  UNIT_TEST_VIRTUAL int is_new_case(int day, int disease) const;
  UNIT_TEST_VIRTUAL int addInfected(int disease, vector<int> strains);
  
  UNIT_TEST_VIRTUAL void infect(Person *infectee, int disease, Transmission *transmission);
  UNIT_TEST_VIRTUAL void getInfected(Disease *disease, Transmission *transmission);
  void addIncidence(int disease, vector<int> strains);
  void addPrevalence(int disease, vector<int> strains);
  
  UNIT_TEST_VIRTUAL void set_changed(); // notify the population that this Person has changed
  void update_demographics(Date *sim_start_date, int day) { demographics->update(sim_start_date, day); }
  void update_health(int day) { health->update(day); }
  void update_behavior(int day) { behavior->update(day); }
  void update_activities(int day) { activities->update(day); }
  void update_activity_profile() { activities->update_profile(); }
  void withdraw_from_activities() { activities->withdraw(); }
  void update_household_mobility() { activities->update_household_mobility(); }

  void become_susceptible(int disease) { health->become_susceptible(disease); }
  void become_exposed(Infection * infection) { health->become_exposed(infection); }
  void become_infectious(Disease * disease) { health->become_infectious(disease); }
  void become_symptomatic(Disease *disease) { health->become_symptomatic(disease); }
  void recover(Disease * disease) { health->recover(disease); }
  bool is_deceased() { return demographics->is_deceased(); }
  
  Person * give_birth(int day);
  void assign_classroom() { activities->assign_classroom(); }
  void assign_office() { activities->assign_office(); }

  //Event Handling
  UNIT_TEST_VIRTUAL void register_event_handler(Person_Event_Interface *event_handler);
  UNIT_TEST_VIRTUAL void deregister_event_handler(Person_Event_Interface *event_handler);
  UNIT_TEST_VIRTUAL void notify_property_change(string property_name, int prev_val, int new_val);
  UNIT_TEST_VIRTUAL void notify_property_change(string property_name, bool new_val);

  // access functions:
  UNIT_TEST_VIRTUAL int get_id() const { return idx; }
  UNIT_TEST_VIRTUAL Population* get_population() const { return pop; }
  UNIT_TEST_VIRTUAL Demographics* get_demographics() const { return demographics; }
  UNIT_TEST_VIRTUAL Health *get_health() const { return health; }
  UNIT_TEST_VIRTUAL Activities* get_activities() const { return activities; }
  UNIT_TEST_VIRTUAL Behavior* get_behavior() const { return behavior; }
  
private:
  int idx;
  Population *pop;
  Demographics *demographics;
  Health *health;
  Activities *activities;
  Behavior *behavior;
  vector<Person_Event_Interface *> *registered_event_handlers;
};

#endif // _FRED_PERSON_H

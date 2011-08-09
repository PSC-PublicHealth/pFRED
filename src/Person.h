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

class Population;
class Place;
class Disease;
class Infection;
class Population;
class Transmission;
#include "Demographics.h"
#include "Health.h"
#include "Behavior.h"
#include "Activities.h"


class Person {
public:
  Person();
  Person(int index, int age, char sex, int marital, int occ, char *house,
	 char *school, char *work, Population *Pop, int day);
  ~Person();
  void newborn_setup(int index, int age, char sex, int marital, int profession,
				       Place **favorite_places, Population* pop, int day);
  void become_unsusceptible(int disease);
  void become_immune(Disease *disease);
  void update_infectious_activities(int day) {
    activities->update_infectious_activities(day);
  }
  void update_susceptible_activities(int day) {
    activities->update_susceptible_activities(day);
  }
  void print(FILE *fp, int disease) const;
  int is_symptomatic() { return health->is_symptomatic(); }
  bool is_susceptible(int dis) { return health->is_susceptible(dis); }
  bool is_infectious(int dis) { return health->is_infectious(dis); }
  int get_diseases();
  
  Place *get_household() const;
  Place *get_neighborhood() const;
  int get_age() const { return demographics->get_age(); }
  int get_init_age() const { return demographics->get_init_age(); }
  int get_init_profession() const { return demographics->get_init_profession(); }
  int get_init_marital_status() const { return demographics->get_init_marital_status(); }
  double get_real_age(int day) const { return demographics->get_real_age(day); }
  char get_sex() const;
  char get_marital_status() const;
  int get_profession() const;
  char get_disease_status(int disease) const { return health->get_disease_status(disease); }
  double get_susceptibility(int disease) const { return health->get_susceptibility(disease); }
  double get_infectivity(int disease, int day) const { return health->get_infectivity(disease, day); }
  int get_exposure_date(int disease) const;
  int get_infectious_date(int disease) const;
  int get_recovered_date(int disease) const;
  int get_infector(int disease) const;
  int get_infected_place(int disease) const;
  char * get_infected_place_label(int disease) const;
  char get_infected_place_type(int disease) const;
  int get_infectees(int disease) const;
  int add_infectee(int disease);
  int is_new_case(int day, int disease) const;
  int addInfected(int disease, vector<int> strains);
  
  void infect(Person *infectee, int disease, Transmission *transmission);
  void getInfected(Disease *disease, Transmission *transmission);
  void addIncidence(int disease, vector<int> strains);
  void addPrevalence(int disease, vector<int> strains);
  
  void set_changed(); // notify the population that this Person has changed
  void update_demographics(int day) { demographics->update(day); }
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
  bool is_adult() { return demographics->get_age() >= Global::ADULT_AGE; }
  
  Person * give_birth(int day);
  void assign_classroom() { activities->assign_classroom(); }
  void assign_office() { activities->assign_office(); }

  // access functions:
  int get_id() const { return idx; }
  Population* get_population() const { return pop; }
  Demographics* get_demographics() const { return demographics; }
  Health *get_health() const { return health; }
  Activities* get_activities() const { return activities; }
  Behavior* get_behavior() const { return behavior; }
  Place * get_household() { return activities->get_household(); }
  Place * get_neighborhood() { return activities->get_neighborhood(); }
  Place * get_school() { return activities->get_school(); }
  Place * get_classroom() { return activities->get_classroom(); }
  Place * get_workplace() { return activities->get_workplace(); }
  Place * get_office() { return activities->get_office(); }
  void start_traveling(Person *visited){ activities->start_traveling(visited); }
  void stop_traveling(){ activities->stop_traveling(); }
  bool get_travel_status(){ return activities->get_travel_status(); }

private:
  int idx;
  Population *pop;
  Demographics *demographics;
  Health *health;
  Activities *activities;
  Behavior *behavior;
};

#endif // _FRED_PERSON_H

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
#include "Cognition.h"
#include "Behavior.h"
#include "Infection.h"
class Place;
class Disease;

class Person {
public:
  Person();
  UNIT_TEST_VIRTUAL ~Person();
  UNIT_TEST_VIRTUAL void setup(int index, int age, char sex, int marital, int profession, Place **favorite_places, int profile, Population* pop);
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL void update_infectious_behavior(int day) { behavior->update_infectious_behavior(day); }
  UNIT_TEST_VIRTUAL void update_susceptible_behavior(int day)  { behavior->update_susceptible_behavior(day); }
  UNIT_TEST_VIRTUAL void print(int disease) const;
  UNIT_TEST_VIRTUAL void print_out(int disease) const;
  UNIT_TEST_VIRTUAL void print_schedule() const;
  UNIT_TEST_VIRTUAL void become_immune(Disease *disease);
  int is_symptomatic() { return health->is_symptomatic(); }
  bool is_susceptible(int dis) { return health->is_susceptible(dis); }
  bool is_infectious(int dis) { return health->is_infectious(dis); }
  int get_diseases() { return pop->get_diseases(); }
  
  // access functions:
  UNIT_TEST_VIRTUAL int get_id() const { return idx; }
  UNIT_TEST_VIRTUAL Place *get_household() const;
  int get_age() const { return demographics->get_age(); }
  UNIT_TEST_VIRTUAL char get_sex() const;
  UNIT_TEST_VIRTUAL char get_marital_status() const;
  UNIT_TEST_VIRTUAL int get_profession() const;
  UNIT_TEST_VIRTUAL char get_disease_status(int disease) const { return health->get_disease_status(disease); }
  UNIT_TEST_VIRTUAL double get_susceptibility(int disease) const { return health->get_susceptibility(disease); }
  UNIT_TEST_VIRTUAL double get_infectivity(int disease) const { return health->get_infectivity(disease); }
  UNIT_TEST_VIRTUAL int get_exposure_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_infectious_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_recovered_date(int disease) const;
  UNIT_TEST_VIRTUAL int get_infector(int disease) const;
  UNIT_TEST_VIRTUAL int get_infected_place(int disease) const;
  UNIT_TEST_VIRTUAL char get_infected_place_type(int disease) const;
  UNIT_TEST_VIRTUAL int get_infectees(int disease) const;
  UNIT_TEST_VIRTUAL int add_infectee(int disease);
  UNIT_TEST_VIRTUAL int is_new_case(int day, int disease) const;
  
  UNIT_TEST_VIRTUAL Health *get_health() const { return health; }
  UNIT_TEST_VIRTUAL Behavior* get_behavior() const { return behavior; }
  UNIT_TEST_VIRTUAL Demographics* get_demographics() const { return demographics; }
  UNIT_TEST_VIRTUAL Cognition* get_cognition() const { return cognition; }
  UNIT_TEST_VIRTUAL Population* get_population() const { return pop; }
  
  UNIT_TEST_VIRTUAL void set_changed(); // notify the population that this Person has changed
  void update_demographics(int day) { demographics->update(day); }
  void update_health(int day) { health->update(day); }
  void update_cognition(int day) { cognition->update(day); }
  void update_behavior(int day) { behavior->update(day); }

  void become_susceptible(int disease) { health->become_susceptible(disease); }
  void become_exposed(Infection * infection) { health->become_exposed(infection); }
  void become_infectious(Disease * disease) { health->become_infectious(disease); }
  void become_symptomatic(Disease *disease) { health->become_symptomatic(disease); }
  void recover(Disease * disease) { health->recover(disease); }
  
private:
  int idx;
  Population *pop;
  Demographics *demographics;
  Health *health;
  Behavior *behavior;
  Cognition *cognition;
};

#endif // _FRED_PERSON_H

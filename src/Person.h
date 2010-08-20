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

#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#include "Behavior.h"
#include "Health.h"

class Demographics;
class Infection;
class Cognition;
class Place;
class Strain;
class Population;

class Person {
public:
  Person();
  virtual ~Person();
  virtual void setup(int index, int age, char sex, int marital, int profession, Place **favorite_places, int profile, Population* pop);
  virtual void reset();
  virtual void update(int day);
  virtual void print(int strain) const;
  virtual void print_out(int strain) const;
  virtual void update_schedule(int day);
  virtual void get_schedule(int *n, Place **sched) const;
  virtual int is_on_schedule(int day, int loc) const { return behavior->is_on_schedule(day, loc); }
  
  virtual void print_schedule() const;
  virtual void become_susceptible(int strain);
  virtual void become_exposed(Infection *infection);
  virtual void become_infectious(Strain *strain);
  virtual void become_symptomatic(Strain *strain);
  virtual void become_immune(Strain *strain);
  virtual void recover(Strain * strain);
  virtual void behave(int day);
  virtual int is_symptomatic() const;
  
  // access functions:
  virtual int get_id() const { return idx; }
  virtual int get_age() const;
  virtual char get_sex() const;
  virtual char get_marital_status() const;
  virtual int get_profession() const;
  virtual int get_places() const;
  virtual char get_strain_status(int strain) const { return health->get_strain_status(strain); }
  virtual double get_susceptibility(int strain) const;
  virtual double get_infectivity(int strain) const;
  virtual int get_exposure_date(int strain) const;
  virtual int get_infectious_date(int strain) const;
  virtual int get_recovered_date(int strain) const;
  virtual int get_infector(int strain) const;
  virtual int get_infected_place(int strain) const;
  virtual char get_infected_place_type(int strain) const;
  virtual int get_infectees(int strain) const;
  virtual int add_infectee(int strain);
  virtual int is_new_case(int day, int strain) const;
  
  virtual Health *get_health() const { return health; }
  virtual Behavior* get_behavior() const { return behavior; }
  virtual Demographics* get_demographics() const { return demographics; }
  virtual Cognition* get_cognition() const { return cognition; }
  virtual Population* get_population() const { return pop; }
  
  virtual void set_changed(); // notify the population that this Person has changed
  
private:
  int idx;
  Population *pop;
  Demographics *demographics;
  Health *health;
  Behavior *behavior;
  Cognition *cognition;
};

#endif // _FRED_PERSON_H

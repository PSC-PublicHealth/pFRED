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
class Perceptions;
class Place;
class Strain;
class Population;

class Person {
public:
  Person(void);
  ~Person(void);
  void setup(int index, int age, char sex, int marital, int occ,
	     int profession, Place *house, Place *neigh,
	     Place *school, Place *classroom, Place *work,
	     Place *office, int profile, Population* pop);
  void reset();
  void update(int day);
  void print(int strain) const;
  void print_out(int strain) const;
  void update_schedule(int day);
  void get_schedule(int *n, int *sched) const;
  inline int is_on_schedule(int day, int loc) const{
    return behavior->is_on_schedule(day, loc);
  }

  void print_schedule() const ;
  void become_susceptible(int strain);
  void become_exposed(Infection *infection);
  void become_infectious(Strain * strain);
  void become_immune(Strain *strain);
  void recover(Strain * strain);
  void behave(int day);
  int is_symptomatic() const;

  // access functions:
  int get_id() const { return id; }
  int get_age() const;
  char get_sex() const;
  char get_occupation() const;
  char get_marital_status() const;
  int get_profession() const;
  int get_places() const;
  inline char get_strain_status(int strain) const {
    return health->get_strain_status(strain);
  }
  double get_susceptibility(int strain) const;
  double get_infectivity(int strain) const;
  int get_exposure_date(int strain) const;
  int get_infectious_date(int strain) const;
  int get_recovered_date(int strain) const;
  int get_infector(int strain) const;
  int get_infected_place(int strain) const;
  char get_infected_place_type(int strain) const;
  int get_infectees(int strain) const;
  int add_infectee(int strain);
  int is_new_case(int day, int strain) const;

  Health *get_health() const             {return health;}
  Behavior* get_behavior() const         {return behavior;}
  Demographics* get_demographics() const {return demographics;}
  Perceptions* get_perceptions() const   {return perceptions;}
  Population* get_population() const     {return pop;}

  void set_changed(void); // notify the population that this Person has changed

private:
  int id;
  Population* pop;
  Demographics * demographics;
  Health * health;
  Behavior * behavior;
  Perceptions * perceptions;
};

#endif // _FRED_PERSON_H

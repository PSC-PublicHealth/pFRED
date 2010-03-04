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

class Person {
public:
  Person() {}
  void setup(int index, int age, char sex, int marital, int occ,
	     int profession, Place *house, Place *neigh,
	     Place *school, Place *classroom, Place *work,
	     Place *office, int profile);
  void reset();
  void update(int day);
  void print(int strain);
  void print_out(int strain);
  void update_schedule(int day);
  void get_schedule(int *n, int *sched);
  inline int is_on_schedule(int day, int loc) {
    return behavior->is_on_schedule(day, loc);
  }

  void print_schedule();
  void become_susceptible(int strain);
  void become_exposed(Infection *infection);
  void become_infectious(Strain * strain);
  void recover(Strain * strain);
  void behave(int day);
  int is_symptomatic();
  const Antiviral* get_av(int strain, int day);

  // access functions:
  int get_id() { return id; }
  int get_age();
  char get_sex();
  char get_occupation();
  char get_marital_status();
  int get_profession();
  int get_places();
  inline char get_strain_status(int strain) {
    return health->get_strain_status(strain);
  }
  double get_susceptibility(int strain);
  double get_infectivity(int strain);
  int get_exposure_date(int strain);
  int get_infectious_date(int strain);
  int get_recovered_date(int strain);
  int get_infector(int strain);
  int get_infected_place(int strain);
  char get_infected_place_type(int strain);
  int get_infectees(int strain);
  int add_infectee(int strain);

  //Health *get_health() { return health;}
  

private:
  int id;
  Demographics * demographics;
  Health * health;
  Behavior * behavior;
  Perceptions * perceptions;
};

#endif // _FRED_PERSON_H

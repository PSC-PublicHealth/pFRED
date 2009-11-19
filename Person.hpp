/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Person.hpp
//

#ifndef _FRED_PERSON_H
#define _FRED_PERSON_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

class Place;
class Infection;
class Strain;

class Person {
public:
  Person() {}

  void setup(int,int,char,int,int,int,Place*,Place*,Place*,
	     Place*,Place*,Place*,int);
  void print(int strain);
  void print_out(int strain);
  void reset();

  void update_schedule(int day);
  void get_schedule(int *n, int *sched);
  int is_on_schedule(int day, int loc);
  void print_schedule();

  // void become_exposed(Strain * strain, int infector,int loc, char place_type, int day);
  void become_exposed(Infection *infection);
  void become_infectious(Strain * strain);
  void recover(Strain * strain);
  void behave(int day);
  int is_symptomatic();

  // access functions:
  int get_id() { return id; }
  int get_age();
  char get_sex();
  char get_occupation();
  char get_marital_status();
  int get_profession();
  int get_places();

  char get_strain_status(int strain);
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

private:
  // member classes:
  #include "Demographics.hpp"
  #include "Health.hpp"
  #include "Behavior.hpp"
  #include "Perceptions.hpp"

  int id;
  Demographics * demographics;
  Health * health;
  Behavior * behavior;
  Perceptions * perceptions;
};

#endif // _FRED_PERSON_H

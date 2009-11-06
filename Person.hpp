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

#define MAX_PLACES 6

#define DAYS_PER_WEEK 7
#define NO_ROLE 'N'
#define HCW 'H'
#define PATIENT 'P'
#define DOCTOR 'D'

#include <stdio.h>
#include <stdlib.h>

class Person {
  int id;
  int age;
  char sex;
  char occupation;
  int marrital_status;
  int profession;

  int profile;				 // index of usual visit pattern
  int favorite_place[MAX_PLACES];	      // list of expected places
  int favorite_places;		   // number of places expected to visit
  char on_schedule[MAX_PLACES]; // 1 = favorite place is on schedule; 0 o.w.
  int schedule[MAX_PLACES];  // list of place ids actually visited today
  int scheduled_places;		 // number places actually visited today
  int schedule_updated;			 // date of last schedule update

  char *disease_status;
  int *latent_period;
  int *infectious_period;
  int *exposure_date;
  int *infectious_date;
  int *recovered_date;
  int *infector;
  int *infected_place;
  char *infected_place_type;
  int *infectees;
  double *susceptibility;
  double *infectivity;
  char *role;

public:
  Person() {}
  void setup(int i, int a, char g, int m, int o, int p, int h,
	     int n, int s, int c, int w, int off, int pro);
  void print(int d);
  void print_out(int d);

  void add_to_favorite_places(int p, int loc) {
    favorite_place[p] = loc; favorite_places++;
  }
  void update_schedule(int day);
  void get_schedule(int *n, int *sched);
  int is_on_schedule(int day, int loc);
  void print_schedule();
  void set_occupation();

  void make_susceptible();
  void make_exposed(int d, int person_id, int loc, char place_type, int day);
  void make_infectious(int d);
  void make_recovered(int d);
  void behave(int day);
  int is_symptomatic();

  // access functions:
  int get_id() { return id; }
  int get_age() { return age; }
  char get_sex() { return sex; }
  char get_occupation() { return occupation; }
  int get_places() { return favorite_places; }
  char get_role(int dis) { return role[dis]; }

  char get_disease_status(int d) { return disease_status[d]; }
  double get_susceptibility(int d) { return susceptibility[d]; }
  double get_infectivity(int d) { return infectivity[d]; }
  int get_exposure_date(int d) { return exposure_date[d]; }
  int get_infectious_date(int d) { return infectious_date[d]; }
  int get_recovered_date(int d) { return recovered_date[d]; }
  int get_infector(int d) { return infector[d]; }
  int get_infected_place(int d) { return infected_place[d]; }
  char get_infected_place_type(int d) { return infected_place_type[d]; }
  int get_infectees(int d) { return infectees[d]; }
  int add_infectee(int d) { return ++infectees[d]; }
  void set_disease_status(int d, char s) { disease_status[d] = s; }
  void set_susceptibility(int d, double x) { susceptibility[d] = x; }
  void set_infectivity(int d, double x) { infectivity[d] = x; }
  void set_infector(int d, int id) { infector[d] = id; }
  void set_role(int dis, char r) { role[dis]= r; }
};

#endif // _FRED_PERSON_H

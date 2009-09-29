//
//
// File: Person.hpp
//

#ifndef _SYNDEM_PERSON_H
#define _SYNDEM_PERSON_H

#define DAYS_PER_WEEK 7

#include <stdio.h>
#include <vector>
using namespace std;

#include "Disease.hpp"
#include "Pop.hpp"
#include "Place.hpp"
class Place;


class Person {
  int id;
  int age;
  char sex;
  char occupation;

  int places;
  vector<Place*> place;
  vector<int> schedule[DAYS_PER_WEEK];

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
  float *susceptibility;
  float *infectivity;

public:
  Person() {};
  void setup(int i, int a, char g);
  void print(int d);
  void print_out(int d);

  void add_to_schedule(int day, int loc, Place *pl);
  int on_schedule(int day, int p);
  void print_schedule();
  int get_schedule_location(int day, int n) { return schedule[day%DAYS_PER_WEEK][n]; }
  int get_schedule_size(int day) { return schedule[day%DAYS_PER_WEEK].size(); }
  void set_occupation();

  void make_susceptible();
  void make_exposed(int d, int person_id, int loc, int day);
  void make_infectious(int d);
  void make_recovered(int d);

  // access functions:
  int get_id() { return id; }
  int get_age() { return age; }
  char get_sex() { return sex; }
  char get_occupation() { return occupation; }
  int get_places() { return places; }

  char get_disease_status(int d) { return disease_status[d]; }
  float get_susceptibility(int d) { return susceptibility[d]; } ;
  float get_infectivity(int d) { return infectivity[d]; } ;
  int get_exposure_date(int d) { return exposure_date[d]; }
  int get_infectious_date(int d) { return infectious_date[d]; }
  int get_recovered_date(int d) { return recovered_date[d]; }
  int get_infector(int d) { return infector[d]; }
  int get_infected_place(int d) { return infected_place[d]; }
  char get_infected_place_type(int d) { return infected_place_type[d]; }
  int get_infectees(int d) { return infectees[d]; }
  int add_infectee(int d) { return ++infectees[d]; }
  void set_disease_status(int d, char s) { disease_status[d] = s; }
  void set_susceptibility(int d, float x) { susceptibility[d] = x; } ;
  void set_infectivity(int d, float x) { infectivity[d] = x; } ;
  void set_infector(int d, int id) { infector[d] = id; }
};

#endif // _SYNDEM_PERSON_H

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Place.h
//

#ifndef _FRED_PLACE_H
#define _FRED_PLACE_H

#include "Global.h"
#include <vector>
#include <set>
#include <iostream>
#include <stdio.h>
#include <limits.h> 

#include "Population.h"

#define HOUSEHOLD 'H'
#define NEIGHBORHOOD 'N'
#define SCHOOL 'S'
#define CLASSROOM 'C'
#define WORKPLACE 'W'
#define OFFICE 'O'
#define HOSPITAL 'M'
#define COMMUNITY 'X'

using namespace std;

class Person;
extern int Diseases;

class Place {
public:
  Place() {}
  UNIT_TEST_VIRTUAL ~Place() {}
  UNIT_TEST_VIRTUAL void setup(int loc_id, const char *lab, double lon, double lat, Place *cont, Population *pop);
  virtual void reset();
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL void print(int disease);
  virtual void add_person(Person * per);
  UNIT_TEST_VIRTUAL void add_visitor(Person * per);
  virtual void add_susceptible(int disease, Person * per);
  virtual void delete_susceptible(int disease, Person * per);
  virtual void add_infectious(int disease, Person * per);
  virtual void delete_infectious(int disease, Person * per);
  UNIT_TEST_VIRTUAL void print_susceptibles(int disease);
  UNIT_TEST_VIRTUAL void print_infectious(int disease);
  virtual void spread_infection(int day, int disease);
  UNIT_TEST_VIRTUAL int is_open(int day);
  
  virtual void get_parameters(int disease) = 0;
  virtual int get_group(int disease, Person * per) = 0;
  virtual double get_transmission_prob(int disease, Person * i, Person * s) = 0;
  virtual double get_contacts_per_day(int disease) = 0; // access functions
  virtual int should_be_open(int day, int disease) { return 1; }
  UNIT_TEST_VIRTUAL int get_id() { return id; }
  UNIT_TEST_VIRTUAL char * get_label() { return label; }
  UNIT_TEST_VIRTUAL int get_type() { return type; }
  UNIT_TEST_VIRTUAL double get_latitude() { return latitude; }
  UNIT_TEST_VIRTUAL double get_longitude() { return longitude; }
  UNIT_TEST_VIRTUAL int get_S(int disease) { return S[disease]; }
  UNIT_TEST_VIRTUAL int get_I(int disease) { return (int) (infectious[disease].size()); }
  UNIT_TEST_VIRTUAL int get_symptomatic(int disease) { return Sympt[disease]; }
  UNIT_TEST_VIRTUAL int get_size() { return N; }
  UNIT_TEST_VIRTUAL int get_close_date() { return close_date; }
  UNIT_TEST_VIRTUAL int get_open_date() { return open_date; }
  UNIT_TEST_VIRTUAL int get_adults() { return adults; }
  UNIT_TEST_VIRTUAL int get_children() { return children; }
  UNIT_TEST_VIRTUAL Population *get_population() { return population; }
  UNIT_TEST_VIRTUAL int get_daily_cases(int disease) { return cases[disease]; }
  UNIT_TEST_VIRTUAL int get_daily_deaths(int disease) { return deaths[disease]; }
  UNIT_TEST_VIRTUAL int get_total_cases(int disease) { return total_cases[disease]; }
  UNIT_TEST_VIRTUAL int get_total_deaths(int disease) { return total_deaths[disease]; }
  UNIT_TEST_VIRTUAL double get_incidence_rate(int disease) { return (double) total_cases[disease] / (double) N; }
  
  UNIT_TEST_VIRTUAL void set_id(int n) { id = n; }
  UNIT_TEST_VIRTUAL void set_type(char t) { type = t; }
  UNIT_TEST_VIRTUAL void set_latitude(double x) { latitude = x; }
  UNIT_TEST_VIRTUAL void set_longitude(double x) { longitude = x; }
  UNIT_TEST_VIRTUAL void set_close_date(int day) { close_date = day; }
  UNIT_TEST_VIRTUAL void set_open_date(int day) { open_date = day; }
  UNIT_TEST_VIRTUAL void set_population(Population *p) {  population = p; }
  UNIT_TEST_VIRTUAL void set_container(Place *cont) { container = cont; }
  UNIT_TEST_VIRTUAL void add_case() { cases++; }
  UNIT_TEST_VIRTUAL void add_deaths() { deaths++; }
  
protected:
  int id;					// place id
  char label[32];				// external id
  char type;				// HOME, WORK, SCHOOL, COMMUNITY
  Place *container;				// id of container place
  double latitude;				// geo location
  double longitude;				// geo location
  int N;			   // total number of potential visitors
  vector <Person *> *susceptibles;	 // list of susceptible visitors
  set <Person *> *infectious;		  // list of infectious visitors
  int *S;					// susceptible count
  int *I;					// infectious count
  int *Sympt;					// symptomatics count
  int close_date;		    // this place will be closed during:
  int open_date;			    //   [close_date, open_date)
  int adults;					// how many adults
  int children;					// how many children
  int * cases;					// symptomatic cases today
  int * deaths;					// deaths today
  int * total_cases;			      // total symptomatic cases
  int * total_deaths;				// total deaths
  Population *population;
  int visit;

  // disease parameters
  double *beta;	       // place-independent transmissibility per contact
};

#endif // _FRED_PLACE_H

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

#include <vector>
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
extern int Strains;

class Place {
public:
  Place() {}
  virtual ~Place() {}
	
  virtual void setup(int loc_id, const char *lab, double lon, double lat, Place *cont, Population *pop);
  virtual void reset();
  virtual void update(int day);
  virtual void print(int strain);
  virtual void add_person(Person * per);
  virtual void add_susceptible(int strain, Person * per);
  virtual void delete_susceptible(int strain, Person * per);
  virtual void print_susceptibles(int strain);
  virtual void add_infectious(int strain, Person * per);
  virtual void delete_infectious(int strain, Person * per);
  virtual void print_infectious(int strain);
  virtual void spread_infection(int day, int strain);
  virtual int is_open(int day);
  
  virtual void get_parameters(int strain) = 0;
  virtual int get_group_type(int strain, Person * per) = 0;
  virtual double get_transmission_prob(int strain, Person * i, Person * s) = 0;
  virtual double get_contacts_per_day(int strain) = 0; // access functions
  virtual int should_be_open(int day, int strain) { return 1; }
	
  virtual int get_id() { return id; }
  virtual char * get_label() { return label; }
  virtual int get_type() { return type; }
  virtual double get_latitude() { return latitude; }
  virtual double get_longitude() { return longitude; }
  virtual int get_S(int strain) { return S[strain]; }
  virtual int get_I(int strain) { return (int) (infectious[strain].size()); }
  virtual int get_symptomatic(int strain) { return Sympt[strain]; }
  virtual int get_size() { return N; }
  virtual int get_close_date() { return close_date; }
  virtual int get_open_date() { return open_date; }
  virtual int get_adults() { return adults; }
  virtual int get_children() { return children; }
  virtual Population *get_population() { return population; }
  virtual int get_daily_cases(int strain) { return cases[strain]; }
  virtual int get_daily_deaths(int strain) { return deaths[strain]; }
  virtual int get_total_cases(int strain) { return total_cases[strain]; }
  virtual int get_total_deaths(int strain) { return total_deaths[strain]; }
  virtual double get_incidence_rate(int strain) { return (double) total_cases[strain] / (double) N; }
  
  virtual void set_id(int n) { id = n; }
  virtual void set_type(char t) { type = t; }
  virtual void set_latitude(double x) { latitude = x; }
  virtual void set_longitude(double x) { longitude = x; }
  virtual void set_close_date(int day) { close_date = day; }
  virtual void set_open_date(int day) { open_date = day; }
  virtual void set_population(Population *p) {  population = p; }
  virtual void set_container(Place *cont) { container = cont; }
  virtual void add_case() { cases++; }
  virtual void add_deaths() { deaths++; }
  
protected:
  int id;					// place id
  char label[32];				// external id
  char type;				// HOME, WORK, SCHOOL, COMMUNITY
  Place *container;				// id of container place
  double latitude;				// geo location
  double longitude;				// geo location
  int N;			   // total number of potential visitors
  vector <Person *> *susceptibles;	 // list of susceptible visitors
  vector <Person *> *infectious;	  // list of infectious visitors
  int *S;					// susceptible count
  int *I;					// infectious count
  int *Sympt;					// symptomatics count
  int close_date;		    // this place will be closed during:
  int open_date;			    //   [close_date, open_date)
  int indiv_types;			   // distinct types of visitors
  int adults;					// how many adults
  int children;					// how many children
  int * cases;					// symptomatic cases today
  int * deaths;					// deaths today
  int * total_cases;			      // total symptomatic cases
  int * total_deaths;				// total deaths
  Population *population;

  // strain parameters
  double *beta;	       // place-independent transmissibility per contact
};

#endif // _FRED_PLACE_H

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

#define HOUSEHOLD 'H'
#define NEIGHBORHOOD 'N'
#define SCHOOL 'S'
#define CLASSROOM 'C'
#define WORKPLACE 'W'
#define OFFICE 'O'
#define HOSPITAL 'M'
#define COMMUNITY 'X'

#include <vector>
using namespace std;

#include "Population.h"
#include "Global.h"
class Patch;
class Person;



class Place {
public:
  Place() {}
  ~Place() {}

  UNIT_TEST_VIRTUAL void setup(int loc_id, const char *lab, double lon, double lat, Place *cont, Population *pop);
  virtual void prepare();
  UNIT_TEST_VIRTUAL void update(int day);
  virtual void print(int disease);
  virtual void enroll(Person * per);
  virtual void add_susceptible(int disease, Person * per);
  virtual void add_infectious(int disease, Person * per, char status);
  UNIT_TEST_VIRTUAL void print_susceptibles(int disease);
  UNIT_TEST_VIRTUAL void print_infectious(int disease);
  virtual void spread_infection(int day, int disease);
  UNIT_TEST_VIRTUAL bool is_open(int day);
  bool is_infectious(int disease) { return I[disease] > 0; }
  
  virtual void get_parameters(int disease) = 0;
  virtual int get_group(int disease, Person * per) = 0;
  virtual double get_transmission_prob(int disease, Person * i, Person * s) = 0;
  virtual double get_contacts_per_day(int disease) = 0; // access functions
  virtual bool should_be_open(int day, int disease) = 0;
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
  Patch * get_patch() { return patch; }
  void set_patch(Patch *p) { patch = p; }
  
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
  int * cases;					// symptomatic cases today
  int * deaths;					// deaths today
  int * total_cases;			      // total symptomatic cases
  int * total_deaths;				// total deaths
  Population *population;
  int diseases;					// number of diseases
  Patch * patch;			     // geo patch for this place

  // disease parameters
  double *beta;	       // place-independent transmissibility per contact
};

#endif // _FRED_PLACE_H

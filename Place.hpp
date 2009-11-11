/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Place.hpp
//

#ifndef _FRED_PLACE_H
#define _FRED_PLACE_H

#include <vector>
#include <iostream>
#include <stdio.h>
using namespace std;

#define HOUSEHOLD 'H'
#define NEIGHBORHOOD 'N'
#define SCHOOL 'S'
#define CLASSROOM 'C'
#define WORKPLACE 'W'
#define OFFICE 'O'
#define HOSPITAL 'M'
#define COMMUNITY 'X'

class Place {

protected:

  int id;					// place id
  char label[32];				// external id
  char type;				// HOME, WORK, SCHOOL, COMMUNITY
  int container;				// id of container place
  double latitude;				// geo location
  double longitude;				// geo location
  int N;			   // total number of potential visitors
  vector <int> *susceptibles;		 // list of susceptible visitors
  vector <int> *infectious;		  // list of infectious visitors
  int *S;					// susceptible count
  int *I;					// infectious count
  int *Sympt;					// symtomatics count
  int close_date;			     // when to close this place
  int open_date;			      // when to open this place
  int indiv_types;			   // distinct types of visitors

  // disease parameters
  double *beta;	       // place-independent transmissibility per contact

public:
  Place() {}
  ~Place() {}
  void setup(int loc, char *lab, double lon, double lat, int cont);
  void reset();
  void print(int dis);
  void add_susceptible(int dis, int per);
  void delete_susceptible(int dis, int per);
  void print_susceptibles(int dis);
  void add_infectious(int dis, int per);
  void delete_infectious(int dis, int per);
  void print_infectious(int dis);
  void spread_infection(int day);
  int is_open(int day);

  virtual void get_parameters() {}
  virtual int get_group_type(int dis, int per) { return 0; }
  virtual double get_transmission_prob(int dis, int i, int s) { return 1.0; }
  virtual int should_be_open(int day, int dis) { return 1; }
  virtual double get_contacts_per_day(int dis) { return 0; }

  // access functions
  int get_id() { return id; }
  char * get_label() { return label; }
  int get_type() { return type; }
  double get_latitude() { return latitude; }
  double get_longitude() { return longitude; }
  int get_S(int dis) { return S[dis]; }
  int get_I(int dis) { return (int) (infectious[dis].size()); }
  int get_symptomatic(int dis) { return Sympt[dis]; }
  int get_size() { return N; }
  int get_close_date() { return close_date; }
  int get_open_date() { return open_date; }

  void set_id(int n) { id = n; }
  void set_type(char t) { type = t; }
  void set_latitude(double x) { latitude = x; }
  void set_longitude(double x) { longitude = x; }
  void set_close_date(int day) { close_date = day; }
  void set_open_date(int day) { open_date = day; }

};

#endif // _FRED_PLACE_H


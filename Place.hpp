//
//
// File: Place.hpp
//

#ifndef _SYNDEM_PLACE_H
#define _SYNDEM_PLACE_H


#include <stack>
#include <vector>
#include <iostream>
#include <stdio.h>
using namespace std;

#include "Random.hpp"
#include "Person.hpp"
#include "Disease.hpp"
#include "Params.hpp"

class Person;

#define HOUSEHOLD 'H'
#define WORKPLACE 'W'
#define SCHOOL 'S'
#define COMMUNITY 'C'

class Place {

public:
  Place();
  ~Place() {};
  virtual void setup(int,char *,double,double,int);
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
  int check_for_school_closure(int day ,int dis);
  virtual int should_be_open(int day, int dis);

  // access functions
  int get_id() { return id; };
  char * get_label() { return label; };
  int get_type() { return type; } ;
  double get_latitude() { return latitude; } ;
  double get_longitude() { return longitude; } ;
  int get_S(int dis) { return S[dis]; } ;
  int get_I(int dis) { return (int) (infectious[dis].size()); } ;
  int get_symptomatic(int dis) { return Sympt[dis]; } ;
  int get_size() { return N; } ;
  int get_close_date() { return close_date; }
  int get_open_date() { return open_date; }

  void set_id(int n) { id = n; };
  void set_type(char n) { type = n; } ;
  void set_latitude(double x) { latitude = x; } ;
  void set_longitude(double x) { longitude = x; } ;
  void set_contact_prob(int dis, double x) { contact_prob[dis] = x; } ;
  void set_contacts_per_day(int dis, int n) { contacts_per_day[dis] = n; } ;
  void set_close_date(int day) { close_date = day; }
  void set_open_date(int day) { open_date = day; }

protected:

  int id;					// place id
  char label[32];				// external id
  char type;				// HOME, WORK, SCHOOL, COMMUNITY
  double latitude;				// geo location
  double longitude;				// geo location
  int N;			   // total number of potential visitors

  double *beta;	       // place-independent transmissibility per contact
  double *contact_prob;		   // local transmissibility per contact
  int *contacts_per_day;		   // number of contacts per day
  vector <int> *susceptibles;		 // list of susceptible visitors
  vector <int> *infectious;		  // list of infectious visitors
  int *S;				  // susceptible count
  int *I;				  // infectious count
  int *Sympt;				  // symtopmatics count

  int close_date;		    // when to close this place
  int open_date;		    // when to open this place
};

#endif // _SYNDEM_PLACE_H


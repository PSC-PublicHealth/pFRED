/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Community.h
//

#ifndef _FRED_COMMUNITY_H
#define _FRED_COMMUNITY_H

#include "Place.h"

extern double * Community_contacts_per_day;
extern double *** Community_contact_prob;
extern int Community_parameters_set;

class Community : public Place {
public: 
  Community() {};
  ~Community() {};
  Community(int, const	char*,double,double,Place *, Population *);
  void reset();
  void get_parameters(int diseases);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  double get_contacts_per_day(int disease);
  void spread_infection(int day, int sstrin);
  Person * get_possible_infectee(int disease, Person * infector, double lat, double lon);
  void add_susceptible(int disease, Person * per);
  void delete_susceptible(int disease, Person * per);
  void add_infectious(int disease, Person * per);
  void delete_infectious(int disease, Person * per);
  double geo_dist(double lat1, double lat2, double lon1, double lon2, int ic);
 private:
  set <Person *> ** infectious_list;	  // list of infectious visitors
  int groups;
};

#endif // _FRED_COMMUNITY_H


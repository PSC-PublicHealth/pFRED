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
  void get_parameters(int strains);
  int get_group_type(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  double get_contacts_per_day(int strain);
  void spread_infection_in_community(int day, int s);
  void static spread(int day, int s);
};

#endif // _FRED_COMMUNITY_H


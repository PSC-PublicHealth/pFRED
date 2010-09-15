/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: School.h
//

#ifndef _FRED_SCHOOL_H
#define _FRED_SCHOOL_H

#include "Place.h"

extern double * school_contacts_per_day;
extern double *** school_contact_prob;
extern char school_closure_policy[];
extern int school_closure_day;
extern double school_closure_threshold;
extern int school_closure_period;
extern int school_closure_delay;
extern int school_parameters_set;

class School : public Place {
public: 
  School() {}
  ~School() {}
  School(int,const char*,double,double,Place *, Population *pop);
  void get_parameters(int strains);
  int get_group(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  int should_be_open(int day, int strain);
  double get_contacts_per_day(int strain);
};

#endif // _FRED_SCHOOL_H

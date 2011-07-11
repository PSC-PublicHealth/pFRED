/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Hospital.h
//

#ifndef _FRED_HOSPITAL_H
#define _FRED_HOSPITAL_H

#include "Place.h"

class Hospital : public Place {
public: 
  Hospital() {}
  ~Hospital() {}
  Hospital(int,const char*,double,double,Place *, Population *);
  void get_parameters(int diseases);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  double get_contacts_per_day(int disease);
  bool should_be_open(int day, int disease) { return true; }

private:
  static double * Hospital_contacts_per_day;
  static double *** Hospital_contact_prob;
  static bool Hospital_parameters_set;
};

#endif // _FRED_HOSPITAL_H


/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Office.h
//

#ifndef _FRED_OFFICE_H
#define _FRED_OFFICE_H

#include "Place.h"

class Office: public Place {
public: 
  Office() {}
  ~Office() {}
  Office(int,const char *,double,double,Place *, Population *pop);
  void get_parameters(int diseases);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  double get_contacts_per_day(int disease);
  bool should_be_open(int day, int disease) { return true; }

private:
  static double * Office_contacts_per_day;
  static double *** Office_contact_prob;
  static bool Office_parameters_set;
};

#endif // _FRED_OFFICE_H


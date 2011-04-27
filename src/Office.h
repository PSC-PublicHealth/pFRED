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

extern double * Office_contacts_per_day;
extern double *** Office_contact_prob;
extern int Office_parameters_set;

class Office: public Place {
  public:
    Office() {}
    ~Office() {}
    Office(int,const char *,double,double,Place *, Population *pop);
    void get_parameters(int diseases);
    int get_group(int disease, Person * per);
    double get_transmission_prob(int disease, Person * i, Person * s);
    double get_contacts_per_day(int disease);
    bool should_be_open(int day, int disease) {
      return true;
      }
  };

#endif // _FRED_OFFICE_H


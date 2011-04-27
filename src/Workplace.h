/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Workplace.h
//

#ifndef _FRED_WORKPLACE_H
#define _FRED_WORKPLACE_H

#include "Place.h"
#include <vector>

extern double * Workplace_contacts_per_day;
extern double *** Workplace_contact_prob;
extern int Workplace_parameters_set;

class Workplace: public Place {
  public:
    Workplace() {
      offices.clear();
      next_office = 0;
      }
    ~Workplace() {}
    Workplace(int, const char *,double,double,Place *, Population *pop);
    void get_parameters(int diseases);
    void prepare();
    int get_group(int disease, Person * per);
    double get_transmission_prob(int disease, Person * i, Person * s);
    double get_contacts_per_day(int disease);
    void setup_offices();
    Place * assign_office(Person *per);
    bool should_be_open(int day, int disease) {
      return true;
      }
  private:
    vector <Place *> offices;
    int next_office;
  };

#endif // _FRED_WORKPLACE_H


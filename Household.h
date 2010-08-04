/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Household.h
//

#ifndef _FRED_HOUSEHOLD_H
#define _FRED_HOUSEHOLD_H

#include "Place.h"
#include "Person.h"

extern double * Household_contacts_per_day;
extern double *** Household_contact_prob;
extern int Household_parameters_set;

class Household: public Place {
public: 
  Household() {}
  ~Household() {}
  Household(int,const char *,double,double,Place *, Population *);
  void get_parameters(int strains);
  int get_group_type(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  double get_contacts_per_day(int strain);
  void add_person(Person * per);
  Person * get_HoH() { return HoH; }
  Person * get_housemate(int i) { return housemate[i]; }
private:
  Person * HoH;
  vector <Person *> housemate;
};

#endif // _FRED_HOUSEHOLD_H


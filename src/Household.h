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
#include "Patch.h"

extern double * Household_contacts_per_day;
extern double *** Household_contact_prob;
extern int Household_parameters_set;

class Household: public Place {
public: 
  Household() {}
  ~Household() {}
  Household(int,const char *,double,double,Place *, Population *);
  void get_parameters(int diseases);
  int get_group(int disease, Person * per);
  double get_transmission_prob(int disease, Person * i, Person * s);
  double get_contacts_per_day(int disease);
  void add_person(Person * per);
  Person * get_HoH() { return HoH; }
  Person * get_housemate(int i) { return housemate[i]; }
  Place * select_neighborhood() { return patch->select_neighbor(); }
private:
  Person * HoH;
  vector <Person *> housemate;
};

#endif // _FRED_HOUSEHOLD_H


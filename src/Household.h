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

class Household: public Place {
public: 
  Household() {}
  ~Household() {}
  Household(int,char*,double,double,int);
  void get_parameters(int strains);
  int get_group_type(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  double get_contacts_per_day(int strain);

  //int get_adults() { return adults; }
  //int get_children() { return children; }
  //Person * get_HoH() { return HoH; }
  //void set_HoH(Person * per) { HoH = per; }
  //void add_person(Person * per);

  // private:
  //int adults;					// how many adults
  //int children;					// how many children
  //Person * HoH;					// head of household

};

#endif // _FRED_HOUSEHOLD_H


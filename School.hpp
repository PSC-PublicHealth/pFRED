/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: School.hpp
//

#ifndef _FRED_SCHOOL_H
#define _FRED_SCHOOL_H

#include "Place.hpp"

class School: public Place {
public: 
  School() {}
  ~School() {}
  School(int,char*,double,double,int);
  void get_parameters(int strains);
  int get_group_type(int strain, int per);
  double get_transmission_prob(int strain, int i, int s);
  int should_be_open(int day, int strain);
  double get_contacts_per_day(int strain);
};

#endif // _FRED_SCHOOL_H


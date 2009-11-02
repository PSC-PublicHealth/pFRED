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

// From Pop.hpp
int get_age(int per);
double get_attack_rate(int dis);

// From Disease.hpp
int get_diseases();

class School: public Place {

public: 
  School() {}
  ~School() {}
  School(int,char*,double,double,int);
  void get_parameters(int diseases);
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  int should_be_open(int day, int dis);
  double get_contacts_per_day(int dis);
};

#endif // _FRED_SCHOOL_H


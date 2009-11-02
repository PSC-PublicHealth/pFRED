/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Classroom.hpp
//

#ifndef _FRED_CLASSROOM_H
#define _FRED_CLASSROOM_H


#include "Place.hpp"

// From Loc.hpp
int location_should_be_open(int loc, int dis, int day);

// From Pop.hpp
int get_age(int);

// From Disease.hpp
int get_diseases();

class Classroom: public Place {

public: 
  Classroom() {}
  ~Classroom() {}
  Classroom(int,char*,double,double,int);
  void get_parameters(int diseases);
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  int should_be_open(int day, int dis);
  double get_contacts_per_day(int dis);
};

#endif // _FRED_CLASSROOM_H


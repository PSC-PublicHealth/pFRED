/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Classroom.h
//

#ifndef _FRED_CLASSROOM_H
#define _FRED_CLASSROOM_H

#include "Place.h"

class Classroom: public Place {

public: 
  Classroom() {}
  ~Classroom() {}
  Classroom(int,char*,double,double,int);
  void get_parameters(int strains);
  int get_group_type(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  int should_be_open(int day, int strain);
  double get_contacts_per_day(int strain);
};

#endif // _FRED_CLASSROOM_H


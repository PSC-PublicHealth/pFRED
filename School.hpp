/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: School.hpp
//

#ifndef _SYNDEM_SCHOOL_H
#define _SYNDEM_SCHOOL_H


#include "Place.hpp"

class School : public Place {

public: 

  School() {};
  ~School() {};
  School(int,char*,double,double);
  void get_parameters();
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  int should_be_open(int day, int dis);
  double get_contacts_per_day(int dis);

};

#endif // _SYNDEM_SCHOOL_H


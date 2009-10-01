/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Hospital.hpp
//

#ifndef _SYNDEM_HOSPITAL_H
#define _SYNDEM_HOSPITAL_H


#include "Place.hpp"

class Hospital : public Place {

public: 

  Hospital() {};
  ~Hospital() {};
  Hospital(int,char*,double,double);
  void get_parameters();
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  double get_contacts_per_day(int dis);

};

#endif // _SYNDEM_HOSPITAL_H


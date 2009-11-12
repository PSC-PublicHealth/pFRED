/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Hospital.hpp
//

#ifndef _FRED_HOSPITAL_H
#define _FRED_HOSPITAL_H

#include "Place.hpp"

class Hospital : public Place {
public: 
  Hospital() {}
  ~Hospital() {}
  Hospital(int,char*,double,double,int);
  void get_parameters(int strains);
  int get_group_type(int strain, int per);
  double get_transmission_prob(int strain, int i, int s);
  double get_contacts_per_day(int strain);
};

#endif // _FRED_HOSPITAL_H


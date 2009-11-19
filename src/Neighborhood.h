/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Neighborhood.h
//

#ifndef _FRED_NEIGHBORHOOD_H
#define _FRED_NEIGHBORHOOD_H

#include "Place.h"

class Neighborhood : public Place {
public: 
  Neighborhood() {};
  ~Neighborhood() {};
  Neighborhood(int,char*,double,double,int);
  void get_parameters(int strains);
  int get_group_type(int strain, Person * per);
  double get_transmission_prob(int strain, Person * i, Person * s);
  double get_contacts_per_day(int strain);

};

#endif // _FRED_NEIGHBORHOOD_H


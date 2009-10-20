/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Neighborhood.hpp
//

#ifndef _SYNDEM_NEIGHBORHOOD_H
#define _SYNDEM_NEIGHBORHOOD_H


#include "Place.hpp"
int get_age(int);

// From Disease.hpp
int get_diseases();

class Neighborhood : public Place {

public: 

  Neighborhood() {};
  ~Neighborhood() {};
  Neighborhood(int,char*,double,double,int);
  void get_parameters(int diseases);
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  double get_contacts_per_day(int dis);

};

#endif // _SYNDEM_NEIGHBORHOOD_H


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Workplace.hpp
//

#ifndef _FRED_WORKPLACE_H
#define _FRED_WORKPLACE_H


#include "Place.hpp"

// From Disease.hpp
int get_diseases();

class Workplace: public Place {

public: 
  Workplace() {}
  ~Workplace() {}
  Workplace(int,char*,double,double,int);
  void get_parameters(int diseases);
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  double get_contacts_per_day(int dis);
};

#endif // _FRED_WORKPLACE_H


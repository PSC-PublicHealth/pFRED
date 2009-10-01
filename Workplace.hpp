/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Workplace.hpp
//

#ifndef _SYNDEM_WORKPLACE_H
#define _SYNDEM_WORKPLACE_H


#include "Place.hpp"

class Workplace : public Place {

public: 

  Workplace() {};
  ~Workplace() {};
  Workplace(int,char*,double,double);
  void get_parameters();
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  double get_contacts_per_day(int dis);

};

#endif // _SYNDEM_WORKPLACE_H


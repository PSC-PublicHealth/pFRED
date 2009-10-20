/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Community.hpp
//

#ifndef _SYNDEM_COMMUNITY_H
#define _SYNDEM_COMMUNITY_H


#include "Place.hpp"
int get_age(int);

// From Disease.hpp
int get_diseases();

class Community : public Place {

public: 

  Community() {};
  ~Community() {};
  Community(int,char*,double,double,int);
  void get_parameters(int diseases);
  int get_group_type(int dis, int per);
  double get_transmission_prob(int dis, int i, int s);
  double get_contacts_per_day(int dis);

};

#endif // _SYNDEM_COMMUNITY_H


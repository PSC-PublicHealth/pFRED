//
//
// File: Household.hpp
//

#ifndef _SYNDEM_HOUSEHOLD_H
#define _SYNDEM_HOUSEHOLD_H


#include "Place.hpp"

class Household : public Place {

public: 

  Household() {};
  Household(int,char*,double,double,int);
  ~Household() {};
  void get_parameters();

};

#endif // _SYNDEM_HOUSEHOLD_H


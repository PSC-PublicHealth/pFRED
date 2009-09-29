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
  Workplace(int,char*,double,double,int);
  ~Workplace() {};
  void get_parameters();
  int should_be_open(int day, int dis);

};

#endif // _SYNDEM_WORKPLACE_H


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
  School(int,char*,double,double,int);
  ~School() {};
  void get_parameters();
  int should_be_open(int day, int dis);

};

#endif // _SYNDEM_SCHOOL_H


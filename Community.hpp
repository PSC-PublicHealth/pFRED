//
//
// File: Community.hpp
//

#ifndef _SYNDEM_COMMUNITY_H
#define _SYNDEM_COMMUNITY_H


#include "Place.hpp"

class Community : public Place {

public: 

  Community() {};
  Community(int,char*,double,double,int);
  ~Community() {};
  void get_parameters();
  int should_be_open(int day, int dis);

};

#endif // _SYNDEM_COMMUNITY_H


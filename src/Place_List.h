/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Place_List.h
//

#ifndef _FRED_PLACE_LIST_H
#define _FRED_PLACE_LIST_H

#include <vector>
#include <map>
using namespace std;

class Place;

class Place_List {
public:
  void setup();
  void reset(int run);
  void update(int day);
  void quality_control();
  void get_parameters();
  Place * get_place(int id);
  Place * get_place_at_position(int i) { return places[i]; }
  void add_place(Place * p);
  int get_number_of_places() { return places.size(); }

private:
  char locfile[80];
  vector <Place *> places;
  map<int, int> place_map;
};


#endif // _FRED_PLACE_LIST_H

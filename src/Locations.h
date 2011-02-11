/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Locations.h
//

#ifndef _FRED_LOCATIONS_H
#define _FRED_LOCATIONS_H

#include <vector>
#include <map>
using namespace std;

class Place;

class Locations {
public:
  void setup();
  void reset(int run);
  void update(int day);
  void location_quality_control();
  void get_parameters();
  Place * get_location(int id);
  Place * get_location_at_position(int i) { return location[i]; }
  void add_location(Place * p);
  int get_locations() { return location.size(); }
  double get_minimum_lat() { return minimum_lat; }
  double get_maximum_lat() { return maximum_lat; }
  double get_minimum_lon() { return minimum_lon; }
  double get_maximum_lon() { return maximum_lon; }

private:
  char locfile[80];
  vector <Place *> location;
  map<int, int> location_map;
  double minimum_lon;
  double maximum_lon;
  double minimum_lat;
  double maximum_lat;
};


#endif // _FRED_LOCATIONS_H

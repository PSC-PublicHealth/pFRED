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

class Place;

class Locations {
public:
  void setup_locations();
  void reset_locations(int run);
  void location_quality_control();
  void get_location_parameters();
  int get_open_status(int loc, int day);
  int location_should_be_open(int loc, int strain, int day);
  Place * get_location(int loc);

private:
  char locfile[80];
  Place ** location;
  int locations;
};


#endif // _FRED_LOCATIONS_H

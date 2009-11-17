/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Locations.hpp
//

#ifndef _FRED_LOCATIONS_H
#define _FRED_LOCATIONS_H

class Place;

class Locations {
public:
  void setup_locations();
  void reset_locations(int run);
  void location_quality_control();
  void process_infectious_locations(int day);
  void get_location_parameters();
  int get_open_status(int loc, int day);
  void add_susceptible_to_place(int id, int strain, int per);
  void delete_susceptible_from_place(int id, int strain, int per);
  void add_infectious_to_place(int id, int strain, int per);
  void delete_infectious_from_place(int id, int strain, int per);
  char get_type_of_place(int id);
  int location_should_be_open(int loc, int strain, int day);
  Place * get_location(int loc);

private:
  char locfile[80];
  Place ** location;
  int locations;
};


#endif // _FRED_LOCATIONS_H

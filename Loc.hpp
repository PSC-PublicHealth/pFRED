/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Loc.hpp
//

#ifndef _FRED_LOC_H
#define _FRED_LOC_H

#include <set>
#include <new>
#include <iostream>

#include "Place.hpp"
#include "School.hpp"
#include "Classroom.hpp"
#include "Workplace.hpp"
#include "Office.hpp"
#include "Community.hpp"
#include "Neighborhood.hpp"
#include "Household.hpp"
#include "Hospital.hpp"

void setup_locations();
void reset_locations(int run);
void location_quality_control();
void process_infectious_locations(int day);
void get_location_parameters();
int get_open_status(int loc, int day);
void add_susceptible_to_place(int id, int dis, int per);
void delete_susceptible_from_place(int id, int dis, int per);
void add_infectious_to_place(int id, int dis, int per);
void delete_infectious_from_place(int id, int dis, int per);
char get_type_of_place(int id);
int location_should_be_open(int loc, int dis, int day);

// From Pop.hpp
void update_schedule(int per, int day);
void get_schedule(int per, int *n, int *schedule);
void insert_into_exposed_list(int d, int p);
void insert_into_infectious_list(int d, int p);
void remove_from_exposed_list(int d, int p);
void remove_from_infectious_list(int d, int p);


#endif // _FRED_LOC_H

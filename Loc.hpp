/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Loc.hpp
//

#ifndef _SYNDEM_LOC_H
#define _SYNDEM_LOC_H

#include <set>
#include <new>
#include <iostream>

#include "Person.hpp"
#include "Place.hpp"
#include "School.hpp"
#include "Workplace.hpp"
#include "Community.hpp"
#include "Household.hpp"
#include "Hospital.hpp"

void setup_locations();
void reset_locations(int run);
void location_quality_control();
void process_infectious_locations(int day);
void get_location_parameters();

#endif // _SYNDEM_LOC_H

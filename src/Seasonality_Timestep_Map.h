/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Seasonality_Timestep_Map.h
// 
//
// Created by Jay DePasse on 06/21/2011
//
// Extends functionality of Timestep_Map to control specification of seasonality conditions.
// Adds ability to seed at a specific geographic locations.
// Based on Shawn Brown's updated version of David Galloway's original
// seeding schedule code.
// The format has been made more explicit (and complicated).
// The first line of the file must declare the type of format that will be used.
// "#line_format" means each record occupies a single space delimited line
// "#structured_format" means that each record will be given in "BoulderIO" format
// see http://stein.cshl.org/boulder/docs/Boulder.html


#ifndef _FRED_SEASONALITY_TIMESTEP_MAP_H
#define _FRED_SEASONALITY_TIMESTEP_MAP_H

#include "Global.h"
#include <stdio.h>
#include <map>
#include <vector>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>

#include "Timestep_Map.h"
#include "Utils.h"

using namespace std;

class Seasonality_Timestep_Map : public Timestep_Map {
  
public:

  Seasonality_Timestep_Map(string _name);
  ~Seasonality_Timestep_Map();
 
  void read_map();
  void read_map_line(ifstream * ts_input);
  void read_map_structured(ifstream * ts_input);

  void print() ;

  class Seasonality_Timestep {

  public:

    Seasonality_Timestep() {
      is_complete = false;
      lat = 51.47795;
      lon = 00.00000;
      seasonalityValue = 0;
      loc = false;
    }

    bool parse_line_format(string tsStr) {

      if ( tsStr.size() <= 0 || tsStr.at(0) == '#' ) { // empty line or comment
        return false;
      } else {
        vector<string> tsVec; 
        size_t p1 = 0;
        size_t p2 = 0;
        while ( p2 < tsStr.npos ) {
          p2 = tsStr.find( " ", p1 );
          tsVec.push_back( tsStr.substr( p1, p2 - p1 ) );
          p1 = p2 + 1;
        }
        int n = tsVec.size(); 
        if ( n < 3 ) {
          Utils::fred_abort("Need to specify at least SimulationDayStart, \
              SimulationDayEnd and SeasonalityValue for Seasonality_Timestep_Map. ");
        } else {
          stringstream( tsVec[0] ) >> simDayStart;
          stringstream( tsVec[1] ) >> simDayEnd;
          stringstream( tsVec[2] ) >> seasonalityValue;
          if ( n >= 3 ) {
            stringstream( tsVec[3] ) >> lat;
            stringstream( tsVec[4] ) >> lon;
            loc = true;
          }
        }
        is_complete = true;   
      }
      return is_complete;
    }

    bool is_applicable(int ts, int offset) {
      int t = ts - offset;
      return t >= simDayStart && t <= simDayEnd;
    }

    double get_seasonality_value() {
      return seasonalityValue;
    }

    bool has_location() {
      return loc;
    }

    double get_lon() { 
      if (loc) { return lon; }
      else { Utils::fred_abort("Tried to access location that was not specified.\
            Calls to get_lon() and get_lat() should be preceeded  by has_location()");}
      return NULL;
    }

    double get_lat() {
      if (loc) { return lat; }
      else { Utils::fred_abort("Tried to access location that was not specified.\
          Calls to get_lon() and get_lat() should be preceeded  by has_location()");}
      return NULL;
    }

    void print() {
      printf("start day = %d, end day = %d, seasonality value = %f\n",simDayStart,simDayEnd,seasonalityValue);
    }

  private:

    int simDayStart, simDayEnd;
    double lat, lon, seasonalityValue;
    
    bool is_complete, loc;

  };

private:

  vector < Seasonality_Timestep * > seasonality_timestep_map;
  
  void insert(Seasonality_Timestep * ct);

public:

  typedef vector < Seasonality_Timestep * >::iterator iterator;

  vector < Seasonality_Timestep * >::iterator begin() {
    return seasonality_timestep_map.begin();
  }

  vector < Seasonality_Timestep * >::iterator end() {
    return seasonality_timestep_map.end();
  }

  
};

#endif // _FRED_SEASONALITY_TIMESTEP_MAP_H

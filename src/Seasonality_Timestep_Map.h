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
#include <utility>

#include "Timestep_Map.h"
#include "Utils.h"
#include "Date.h"

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
      simDayRanges.clear();
    }

    bool parse_line_format(string tsStr) {

      int simDayStart, simDayEnd;

      if ( tsStr.size() <= 0 || tsStr.at(0) == '#' ) { // empty line or comment
        return false;
      }
      else {
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
          if (tsVec[0].find('-') == 2 && tsVec[1].find('-') == 2) {
            // start and end specify only MM-DD (repeat this calendar date's value every year)
            parseMMDD(tsVec[0], tsVec[1]);
          }
          else {
            // start and end specified as (integer) sim days
            stringstream( tsVec[0] ) >> simDayStart;
            stringstream( tsVec[1] ) >> simDayEnd;
            simDayRanges.push_back( pair <int,int> (simDayStart,simDayEnd) );
          }
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
      for (int i = 0; i < simDayRanges.size(); i++) {
        if ( t >= simDayRanges[i].first && t <= simDayRanges[i].second ) {
          return true;
        }
      }
      return false;
      //return t >= simDayStart && t <= simDayEnd;
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
      for (int i = 0; i < simDayRanges.size(); i++) {
        printf("start day = %d, end day = %d, seasonality value = %f\n",simDayRanges[i].first,simDayRanges[i].second,seasonalityValue);
      }
    }

  private:

    bool parseMMDD(string startMMDD, string endMMDD) {
      int years = 0.5 + (Global::Days / 365);    
      int startYear = Date::get_current_year(0);
      string dateFormat = string("YYYY-MM-DD");
      for (int y = startYear; y <= startYear + years; y++) {

        stringstream ss_s;
        stringstream ss_e;
        stringstream ss_s2;
        stringstream ss_e2;

        ss_s << y << "-" << startMMDD;
        string startDate = ss_s.str();
        ss_e << y << "-" << endMMDD;
        string endDate = ss_e.str();
        
        if (!(Date::is_leap_year(y))) {
          if (Date::parse_month_from_date_string(startDate,dateFormat) == 2 && Date::parse_day_of_month_from_date_string(startDate,dateFormat) == 29) {
            ss_s2 << y << "-" << "03-01";
            startDate = ss_s2.str();
          }
          if (Date::parse_month_from_date_string(endDate,dateFormat) == 2 && Date::parse_day_of_month_from_date_string(endDate,dateFormat) == 29) {
            ss_e2 << y << "-" << "03-01";
            endDate = ss_e2.str();
          }
        }

        int simStartDay = Date::days_between(0, new Date::Date(startDate,dateFormat));
        int simEndDay = Date::days_between(0, new Date::Date(endDate,dateFormat));
      
        simDayRanges.push_back( pair <int,int> (simStartDay,simEndDay) );

      }
      return true;
    }

    //int simDayStart, simDayEnd;
    vector < pair <int,int> > simDayRanges;
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


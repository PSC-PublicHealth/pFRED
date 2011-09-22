/*
 *  Seasonality_Timestep_Map.cpp
 *  FRED
 *
 *  Created by Jay DePasse on 06/21/2011.
 *  Copyright 2010 University of Pittsburgh. All rights reserved.
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include "Seasonality_Timestep_Map.h"
#include "Params.h"
#include "Utils.h"

using namespace std;

Seasonality_Timestep_Map::Seasonality_Timestep_Map(string _name) : Timestep_Map(_name) {
}

Seasonality_Timestep_Map::~Seasonality_Timestep_Map() {
}


void Seasonality_Timestep_Map::read_map() {
  ifstream * ts_input = new ifstream(map_file_name);

  if(!ts_input->is_open()) {
    Utils::fred_abort("Help!  Can't read %s Timestep Map\n", map_file_name);
    abort();
  }
 
  string line;
  if ( getline(*ts_input,line) ) {
    if ( line == "#line_format" ) {
      read_map_line(ts_input);
    }
    else if ( line == "#structured_format" ) {
      read_map_structured(ts_input);
    }
    else {
      Utils::fred_abort("First line has to specify either #line_format or #structured_format; see primary_case_schedule-0.txt for an example. ");
    }
  } else { 
    Utils::fred_abort("Nothing in the file!");
  }
}

void Seasonality_Timestep_Map::read_map_line(ifstream * ts_input) {
  // There is a file, lets read in the data structure.
  values = new map<int,int>;
  string line;
  while(getline(*ts_input,line)){
    Seasonality_Timestep * cts = new Seasonality_Timestep();
    if (cts->parse_line_format(line)) { insert(cts); } 
  }
}

void Seasonality_Timestep_Map::read_map_structured(ifstream * ts_input) {
}

void Seasonality_Timestep_Map::insert(Seasonality_Timestep * ct) {
   seasonality_timestep_map.push_back(ct);
}

void Seasonality_Timestep_Map::print() {
  cout << "\n";
  cout << name << " Seasonality_Timestep_Map size = " << (int) seasonality_timestep_map.size() <<"\n";
  vector < Seasonality_Timestep * >::iterator itr;
  for(itr=begin(); itr!=end(); ++itr){
    (*itr)->print(); 
  }
  cout << "\n";
}

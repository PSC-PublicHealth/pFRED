/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Timestep_Map.h
//

#ifndef _FRED_TIMESTEP_MAP_H
#define _FRED_TIMESTEP_MAP_H

#include <stdio.h>
#include <map>

using namespace std;

class Timestep_Map {
  // The Day Map class is an input structure to hold a list of values
  // that are indexed by timesteps, this is used for values that 
  // need to be changed on certain timesteps throughout the simulation
  // Based on David Galloway's initial seeding.
  
  // A Timestep_Map is to be specified in a seperate file
  // If a Timestep_Map is specified with [] on the end, it is assumed to
  // be strain specific 
  
  // This structure is designed so that if there is a specification in the 
  // input that specifies "none" as the keyword, the structure is empty.
  
  // The input keyword will be the name of the structure_file.  So if the
  // name of the Map is passed as "primary_cases", the param keyword will
  // be "primary_cases_file".
  
public:
  Timestep_Map();	
  Timestep_Map(string _name);
  ~Timestep_Map();
  
  // Utility Members
  virtual int get_value_for_timestep(int ts) const; // returns the value for the given timestep
  virtual bool is_empty() const { return values->empty(); }
  virtual void print() const;
private:
  map <int, int>* values;  // Map structure that holds <ts, value>
  string name;             // Name of the map
};

#endif _FRED_TIMESTEP_MAP_H

/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Age_Map.h
//
// Age_Map is a class that holds a set of age-specific ranged values
// The age ranges must be mutually exclusive.
//

#ifndef _FRED_AGEMAP_H
#define _FRED_AGEMAP_H

#include "Global.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>

#include "Params.h"

using namespace std;

class Age_Map {
public:
  // Creation operations
  Age_Map(string Name);
  Age_Map();
  
  int get_num_ages() const { return ages.size(); }
  int get_minimum_age() const;
  int get_maximum_age() const;
  
  bool is_empty() const { return ages.empty(); }
  
  // Additional creation operations for building an Age_Map
  void read_from_input(string Input);
  void read_from_input(string Input, int i);
  void read_from_input(string Input, int i, int j);
  void add_value(int lower_age, int upper_age, double val);
  
  // Operations
  double find_value(int age) const;
  
  // Utility functions
  void print() const;
  bool quality_control() const;
  
private:
  string Name;
  vector < vector<int> > ages;  // vector to hold the age ranges
  vector <double> values;       // vector to hold the values for each age range
};

#endif

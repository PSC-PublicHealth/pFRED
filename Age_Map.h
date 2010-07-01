/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AgeMap.h
//

#ifndef _FRED_AGEMAP_H
#define _FRED_AGEMAP_H

#include <stdio.h>
#include <vector>
#include <string>
using namespace std;
#include "Params.h"

class Age_Map{
  //Age_Map is a class that holds a set of age specific ranged values
  // The age ranges much be mutually exclusive.
 public:
  //Creation Operators
  Age_Map(string Name);
  Age_Map();
  
  int get_num_ages(void) const { return ages.size(); }
  bool is_empty(void)    const { return ages.empty();}
  
  //Additional Creation Operations for building an Age Map
  void read_from_input(string Input);
  void add_value(int lower_age,int upper_age,double val);
  
  //Operations
  double find_value(int age) const;
  
  //Utility functions
  void print(void) const ;
  bool quality_control(void) const;
  
 private:
  string Name;
  vector < vector <int> > ages;  // Vector to hold the age rangesxs
  vector < double > values;      // Vector to hold the values for each age range
};

#endif

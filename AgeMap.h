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

class AgeMap{
 public:
  AgeMap(string Name);
  AgeMap();
  
  //Additional Creation Operations
  int read_from_input(string Input);
  void add_value(int lower_age,int upper_age,double val);
  
  //Operations
  double find_value(int age);
  
  //Utility functions
  void print(void);
  
 private:
  string Name;
  vector < vector <int> > ages;
  vector < double > values;
  
};

#endif

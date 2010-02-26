/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Antiviral.h
//
#ifndef _FRED_ANTIVIRALS_H
#define _FRED_ANTIVIRALS_H

#include <stdio.h>
#include <iomanip>
#include <vector>
using namespace std;
#include "Antiviral.h"

class Antiviral;

class Antivirals {
 public:
  Antivirals(void); // This is created from input
  void setup(void); // Need this
  int do_av(void) { return AVs.size() > 0; }
  int get_prophylaxis_start_date() { return prophylaxis_start_date;}
  
  //int update(Population Pop, int day); // Dole out the goods
  
  int get_number_of_AVs() { return AVs.size(); }
  int get_percent_symptomatics_given() { return percent_symptomatics_given;}
  int get_total_current_stock(){
    int sum = 0;
    for(unsigned int i=0;i<AVs.size();i++) sum += AVs[i]->get_stock();
    return sum;
  }
  Antiviral *get_AV(int nav){ return AVs[nav]; }
  
  // Utility Functions
  void print(void);
  void reset(void);
  void report(int day);
  void quality_control(int nstrains);
  int give_which_AV(int strain);
 private:
  vector < Antiviral* > AVs;
  int percent_symptomatics_given;
  FILE* reportFile;  // Create an AV report file
  
  // place holder for Shawn's AV strategy work.
  static int prophylaxis_start_date;

};

#endif // _FRED_ANTIVIRALS_H

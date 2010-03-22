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
  int do_av(void) { return AVs.size() > 0; }
  
  int get_number_antivirals() { return AVs.size(); }
  int get_percent_symptomatics_given() { return percent_symptomatics_given;}
  int get_total_current_stock(){
    int sum = 0;
    for(unsigned int i=0;i<AVs.size();i++) sum += AVs[i]->get_current_stock();
    return sum;
  }
  vector <Antiviral *> get_AV_vector(void){ return AVs; }
  Antiviral* get_AV(int nav){ return AVs[nav]; }
  
  // Utility Functions
  void print(void);
  void print_stocks(void);
  void reset(void);
  void update(int day);
  void report(int day);
  void quality_control(int nstrains);
  vector < Antiviral*> find_applicable_AVs(int strain);
  vector < Antiviral*> prophylaxis_AVs(void);

 private:
  vector < Antiviral* > AVs;
  int percent_symptomatics_given;
  FILE* reportFile;  // Create an AV report file
};

#endif // _FRED_ANTIVIRALS_H

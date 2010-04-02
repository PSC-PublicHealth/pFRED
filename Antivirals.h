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

class Antiviral;

class Antivirals {
  // Antivirals is a class to contain a group of Antiviral classes
 public:
  // Creation Operations
  Antivirals(void);          // This is created from input
  
  //Paramter Access Members
  bool do_av(void)                         const { return AVs.size() > 0; } 
  int get_number_antivirals(void)          const { return AVs.size(); }
  int get_total_current_stock(void)        const;
  vector <Antiviral *> get_AV_vector(void) const { return AVs; }
  Antiviral* get_AV(int nav)               const { return AVs[nav]; }
  
  // Utility Functions
  void print(void);
  void print_stocks(void);
  void reset(void);
  void update(int day);
  void report(int day);
  void quality_control(int nstrains);

  // Polling the collection 
  vector < Antiviral*> find_applicable_AVs(int strain);
  vector < Antiviral*> prophylaxis_AVs(void);

 private:
  vector < Antiviral* > AVs;  // A Vector to hold the AVs in the collection
  FILE* reportFile;           // Create an AV report file, to be deprecated
};
#endif // _FRED_ANTIVIRALS_H

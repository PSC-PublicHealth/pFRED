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
#define MAX_ANTIVIRALS 4

using namespace std;

class Antiviral;

class Antivirals {
  // Antivirals is a class to contain a group of Antiviral classes
public:
  // Creation Operations
  Antivirals();          // This is created from input
  
  //Paramter Access Members
  bool do_av()                               const { return AVs.size() > 0; } 
  int get_number_antivirals()                const { return AVs.size(); }
  int get_total_current_stock()              const;
  vector <Antiviral *> const get_AV_vector() const { return AVs; }
  Antiviral* const get_AV(int nav)           const { return AVs[nav]; }
  
  // Utility Functions
  void print() const;
  void print_stocks() const;
  void reset();
  void update(int day);
  void report(int day) const;
  void quality_control(int nstrains) const;
  
  // Polling the collection 
  vector < Antiviral*> find_applicable_AVs(int strain) const;
  vector < Antiviral*> prophylaxis_AVs() const;
  
private:
  vector < Antiviral* > AVs;  // A Vector to hold the AVs in the collection
  FILE* reportFile;           // Create an AV report file, to be deprecated
};
#endif // _FRED_ANTIVIRALS_H

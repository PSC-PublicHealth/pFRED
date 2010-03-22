/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: VaccineDose.h
//

#ifndef _FRED_VACCINE_H
#define _FRED_VACCINE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
using namespace std;
//#include "AgeMap.h"
//#include "VaccineDose.h"

class Vaccine_Dose;
class AgeMap;

class Vaccine{
 public:
  Vaccine(string nm, int id, int str, int low_age, int high_age, int totav, int addpday, int stday);
  void Add_dose(Vaccine_Dose dose);
  
  int get_strain(void)   { return strain; }
  int get_ID(void)       { return ID; }
  int get_number_doses(void) { return number_doses; }
  int get_low_age(void);
  int get_high_age(void);
  Vaccine_Dose *get_dose(int i){ return &Doses[i]; }
  
  // Logistics Functions
  int get_initial_stock(void)     { return initial_stock; }
  int get_total_avail(void)       { return total_avail; }
  int get_current_reserve(void)   { return reserve; }
  int get_current_stock(void)     { return stock; }
  int get_additional_per_day(void){ return additional_per_day;}
  void add_stock( int add ){ 
    if(add <= reserve){
      stock   += add;
      reserve -= add;
    }
    else{
      stock   += add-reserve;
      reserve  = 0;
    }
  }
  
  void remove_stock( int remove ) {
    stock-=remove;
    if(stock < 0) stock = 0;
  }
  
  //Utility Functions
  void print(void);
  void update(int day);
  void reset(void);

 private:
  string Name;
  int ID;                              // Which in the number of vaccines is it
  int strain;                          // Which Strain is this vaccine for
  int number_doses;                    // How many doses does the vaccine need.
  int ages[2];                         // Applicable Ages
  vector < Vaccine_Dose > Doses;       // Data structure to hold the efficacy of each dose.
  
  int initial_stock;                   // How much available at the beginning
  int total_avail;                     // How much total in reserve
  int stock;                           // How much do you currently have
  int reserve;                         // How much is still left to system
  int additional_per_day;              // How much can be introduced into the system on a given day
  
  int start_day;                       // When to start production
  
  // for statistics
  int number_delivered;
  int number_effective;
};

#endif

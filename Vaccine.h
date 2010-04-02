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

class Vaccine_Dose;

class Vaccine{
 public:
  // Creation
  Vaccine(string _name, int _id, int _strain, int _age0, int _age1, 
	  int _total_avail, int _additional_per_day, int _start_day);
  ~Vaccine(void);
  
  void add_dose(Vaccine_Dose* dose);
  
  int get_strain(void)             const { return strain; }
  int get_ID(void)                 const { return id; }
  int get_number_doses(void)       const { return doses.size(); }
  int get_low_age(void) const;
  int get_high_age(void) const;
  Vaccine_Dose* get_dose(int i)    const { return doses[i]; }
  
  // Logistics Functions
  int get_initial_stock(void)      const { return initial_stock; }
  int get_total_avail(void)        const { return total_avail; }
  int get_current_reserve(void)    const { return reserve; }
  int get_current_stock(void)      const { return stock; }
  int get_additional_per_day(void) const { return additional_per_day; }
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
  string name;
  int id;                              // Which in the number of vaccines is it
  int strain;                          // Which Strain is this vaccine for
  int number_doses;                    // How many doses does the vaccine need.
  int ages[2];                         // Applicable Ages
  vector < Vaccine_Dose* > doses;       // Data structure to hold the efficacy of each dose.
  
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

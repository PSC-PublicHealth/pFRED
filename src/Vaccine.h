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

#include "Global.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class Vaccine_Dose;

class Vaccine{
public:
  // Creation
  Vaccine(string _name, int _id, int _disease, int _age0, int _age1, 
          int _total_avail, int _additional_per_day, int _start_day);
  UNIT_TEST_VIRTUAL ~Vaccine();
  
  UNIT_TEST_VIRTUAL void add_dose(Vaccine_Dose* dose);
  
  UNIT_TEST_VIRTUAL int get_disease()             const { return disease; }
  UNIT_TEST_VIRTUAL int get_ID()                 const { return id; }
  UNIT_TEST_VIRTUAL int get_number_doses()       const { return doses.size(); }
  UNIT_TEST_VIRTUAL int get_low_age() const { return ages[0]; }
  UNIT_TEST_VIRTUAL int get_high_age() const { return ages[1]; }
  UNIT_TEST_VIRTUAL Vaccine_Dose* get_dose(int i)    const { return doses[i]; }
  
  // Logistics Functions
  UNIT_TEST_VIRTUAL int get_initial_stock()      const { return initial_stock; }
  UNIT_TEST_VIRTUAL int get_total_avail()        const { return total_avail; }
  UNIT_TEST_VIRTUAL int get_current_reserve()    const { return reserve; }
  UNIT_TEST_VIRTUAL int get_current_stock()      const { return stock; }
  UNIT_TEST_VIRTUAL int get_additional_per_day() const { return additional_per_day; }
  UNIT_TEST_VIRTUAL void add_stock( int add ){ 
    if(add <= reserve){
      stock   += add;
      reserve -= add;
    }
    else{
      stock   += reserve;
      reserve  = 0;
    }
  }
  
  UNIT_TEST_VIRTUAL void remove_stock( int remove ) {
    stock-=remove;
    if(stock < 0) stock = 0;
  }
  
  //Utility Functions
  UNIT_TEST_VIRTUAL void print() const;
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL void reset();
  
private:
  string name;
  int id;                              // Which in the number of vaccines is it
  int disease;                          // Which Disease is this vaccine for
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
	
protected:
	Vaccine() { }
};

#endif

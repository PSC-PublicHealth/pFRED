/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Vaccines.h
//

#ifndef _FRED_VACCINES_H
#define _FRED_VACCINES_H

#include "Vaccine.h"
#include "VaccineDose.h"

//class Vaccine;
//class Vaccine_Dose;

class Vaccines{
 public:
  Vaccines(string vaccFile);
  
  Vaccine *get_vaccine(int i){ return &vaccines[i];}
    
  void update(int day);
  vector <int> which_vaccines_applicable(int age);
  int pick_from_applicable_vaccines(int age);
  int choose_vaccine(int age);
  int get_total_vaccines_avail_today(void);
  
  
  //utility Functions
  void print(void);
  void print_current_stocks(void);
  void reset(void);
 private:
  vector < Vaccine > vaccines;
}; 

#endif

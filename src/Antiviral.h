/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Antiviral.h
//
#ifndef _FRED_ANTIVIRAL_H
#define _FRED_ANTIVIRAL_H

#include <vector>
#include <stdio.h>
#include <iostream>

using namespace std;

class Strain;
class Health;

class Antiviral {
 public:
  //Creation 
  Antiviral(int s, int cl, double ri, double rs, double rip, 
	    int st, double eff);
  // int administer(Health *h, int day);  //returns 1 if already on treatment;
  //Strain *get_strain {return str
  int     get_strain() { return strain;}
  double  get_reduce_infectivity() { return reduce_infectivity;}
  double  get_reduce_susceptibility() { return reduce_susceptibility;}
  double  get_reduce_infectious_period() { return reduce_infectious_period;}
  int     get_course_length() { return course_length; }
  double  get_percent_symptomatics() { return percent_symptomatics;}
  double  get_efficacy() { return efficacy; }
  

  // Roll operators
  int roll_sympt();
  int roll_efficacy();
  
  // Can define these as operators;
  void reduce_stock(int amount) { stock --;}
  void add_stock(int amount) { stock++;}
  int get_stock() { return stock; }
  int get_init_stock() { return initial_stock;}
  void add_given_out(int amount){ given_out+=amount;}
  void add_ineff_given_out(int amount){ ineff_given_out+=amount;}
  // Utility Functions
  void print(void);
  void reset(void);
  void report(int day);
  int quality_control(int nstrains);

 private:
  int strain;                     // Not sure how to expose.
  int course_length;              // How many days mush one take the AV
  double reduce_infectivity;      // What percentage does it reduce infectivity
  double reduce_susceptibility;   // What percentage does it reduce susceptability
  double reduce_infectious_period;// What percentage does it reduce the infectous period
  double percent_symptomatics;    // Percentage of symptomatics recieving this drug
  double efficacy;                // The effectiveness of the AV (resistance)

  int initial_stock;
  int stock;
  int given_out;
  int ineff_given_out;
  
  //Need to reduce Hosp outcomes;
};
#endif // _FRED_ANTIVIRAL_H

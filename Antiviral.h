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
  Antiviral(int s, int cl, double ri, double rs, double reduce_asymp_period,
	    double reduce_symp_period, double prob_symptoms,
	    int st, double eff, double* av_start_day, int max_av_start_day);

  ~Antiviral() {delete av_start_day;}

  int     get_strain() const { return strain;}
  double  get_reduce_infectivity() const { return reduce_infectivity;}
  double  get_reduce_susceptibility() const { return reduce_susceptibility;}
  double  get_reduce_asymp_period() const { return reduce_asymp_period;}
  double  get_reduce_symp_period() const { return reduce_symp_period;}
  double  get_prob_symptoms() const { return prob_symptoms;}
  int     get_course_length() const { return course_length; }
  double  get_percent_symptomatics() const { return percent_symptomatics;}
  double  get_efficacy() const { return efficacy; }
  

  // Roll operators
  int roll_will_have_symp() const;
  int roll_efficacy() const;
  int roll_start_day() const;
  
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
  double reduce_infectious_period;
  double percent_symptomatics;    // Percentage of symptomatics recieving this drug
  double reduce_asymp_period;     // What percentage does it reduce the asymptomatic period
  double reduce_symp_period;      // What percentage does it reduce the symptomatic period
  double prob_symptoms;           // What is the probability of being symptomatic
  double efficacy;                // The effectiveness of the AV (resistance)
  double* av_start_day;           // Probabilistic AV start
  int max_av_start_day;

  int initial_stock;
  int stock;
  int given_out;
  int ineff_given_out;
  
  //Need to reduce Hosp outcomes;
};
#endif // _FRED_ANTIVIRAL_H

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
class Policy;
class AV_Status;

class Antiviral {
 public:
  //Creation 
  Antiviral(int Strain, int CorLength, double RedInf, 
	    double RedSuc, double RedASympPer, double RedSumpPer,
	    double ProbSymp, int InitSt, int TotAvail, int PerDay, double Eff, 
	    double* AVCourseSt, int MaxAVCourseSt,
	    int StrtDay, int Proph, double PerSympt);
 
  ~Antiviral() {delete av_course_start_day;}

  int     get_strain() const { return strain;}
  double  get_reduce_infectivity() const { return reduce_infectivity;}
  double  get_reduce_susceptibility() const { return reduce_susceptibility;}
  double  get_reduce_asymp_period() const { return reduce_asymp_period;}
  double  get_reduce_symp_period() const { return reduce_symp_period;}
  double  get_prob_symptoms() const { return prob_symptoms;}
  int     get_course_length() const { return course_length; }
  double  get_percent_symptomatics() const { return percent_symptomatics;}
  double  get_efficacy() const { return efficacy; }
  int     get_start_day() { return start_day; }
  int     is_prophylaxis() { return prophylaxis;}
  
  // Roll operators
  int roll_will_have_symp() const;
  int roll_efficacy() const;
  int roll_course_start_day() const;
  
  // Logistics Functions
  int get_initial_stock(void)     { return initial_stock; }
  int get_total_avail(void)       { return total_avail; }
  int get_current_reserve(void)   { return reserve; }
  int get_current_stock(void)     { return stock; }
  int get_additional_per_day(void){ return additional_per_day;}
  void add_stock( int amount ) {
    if(amount < reserve){
      stock += amount;
      reserve -= amount;
    }
    else{
      stock += reserve;
      reserve = 0;
    }
  }
  
  void remove_stock(int remove){
    stock -= remove;
    if(stock < 0) stock = 0;
  }
  
  void add_given_out(int amount){ given_out+=amount;}
  void add_ineff_given_out(int amount){ ineff_given_out+=amount;}
  // Utility Functions
  void update(int day);
  void print(void);
  void reset(void);
  void report(int day);
  int quality_control(int nstrains);
  void print_stocks(void);

  //Effect the Health of Person
  void effect(Health *h, int cur_day, AV_Status* av_stats);
  // void remove_effect(Health *h, int s, int cur_day);

  // Policies tools
  // Antivirals need a policy associated with them to determine who gets them.
  void set_policy(Policy* p) { policy = p; }
  Policy* get_policy() { return policy; }
    
 private:
  int strain;                    
  int course_length;              // How many days mush one take the AV
  double reduce_infectivity;      // What percentage does it reduce infectivity
  double reduce_susceptibility;   // What percentage does it reduce susceptability
  double reduce_infectious_period;
  double percent_symptomatics;    // Percentage of symptomatics recieving this drug
  double reduce_asymp_period;     // What percentage does it reduce the asymptomatic period
  double reduce_symp_period;      // What percentage does it reduce the symptomatic period
  double prob_symptoms;           // What is the probability of being symptomatic
  double efficacy;                // The effectiveness of the AV (resistance)
  double* av_course_start_day;           // Probabilistic AV start
  int max_av_course_start_day;

  //Logistics
  int initial_stock;
  int stock;
  int additional_per_day;
  int total_avail;
  int reserve;
  int start_day;
  
  
  //Policy variables
  int prophylaxis;
  int percent_of_symptomatics;
  
  // For Statistics
  int given_out;
  int ineff_given_out;
  Policy *policy;
  
  //Need to reduce Hosp outcomes;
};
#endif // _FRED_ANTIVIRAL_H

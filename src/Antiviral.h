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
class AV_Health;

class Antiviral {
  //Antiviral is a class to hold all of the paramters to describe a 
  //single antiviral
 public:
  //Creation
  
  Antiviral(int _strain, int _course_length, double _reduce_infectivity,
	    double _reduce_susceptibility, double _reduce_asymp_period,
	    double _reduce_sympt_period, double _prob_symptoms,
	    int _initial_stock, int _total_avail, int _additional_per_day,
	    double _efficacy, double* _av_cousre_start_day, 
	    int _max_av_course_start_day, int _start_day, bool _prophylaxis,
	    double _percent_symptomatics);
  
  ~Antiviral() {delete av_course_start_day;}

  //Paramter Access Members
  int     get_strain()                const { return strain;}
  double  get_reduce_infectivity()    const { return reduce_infectivity;}
  double  get_reduce_susceptibility() const { return reduce_susceptibility;}
  double  get_reduce_asymp_period()   const { return reduce_asymp_period;}
  double  get_reduce_symp_period()    const { return reduce_symp_period;}
  double  get_prob_symptoms()         const { return prob_symptoms;}
  int     get_course_length()         const { return course_length; }
  double  get_percent_symptomatics()  const { return percent_symptomatics;}
  double  get_efficacy()              const { return efficacy; }
  int     get_start_day()             const { return start_day; }
  bool    is_prophylaxis()            const { return prophylaxis;}
  
  // Roll operators
  int roll_will_have_symp()           const;
  int roll_efficacy()                 const;
  int roll_course_start_day()         const;
  
  // Logistics Functions
  int get_initial_stock(void)         const { return initial_stock; }
  int get_total_avail(void)           const { return total_avail; }
  int get_current_reserve(void)       const { return reserve; }
  int get_current_stock(void)         const { return stock; }
  int get_additional_per_day(void)    const { return additional_per_day;}
  
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
  
  // Utility Functions
  void update(int day);
  void print(void)                   const;
  void reset(void);
  void report(int day)               const;
  int  quality_control(int nstrains) const;
  void print_stocks(void)            const ;

  //Effect the Health of Person
  void effect(Health *h, int cur_day, AV_Health* av_health);

  // Policies members
  // Antivirals need a policy associated with them to determine who gets them.
  void set_policy(Policy* p)       {policy = p; }
  Policy* get_policy() const          {return policy;}
  
  // To Be depricated 
  void add_given_out(int amount)            {given_out+=amount;}
  void add_ineff_given_out(int amount)      {ineff_given_out+=amount;}
 
 private:
  int strain;                    
  int course_length;               // How many days mush one take the AV
  double reduce_infectivity;       // What percentage does it reduce infectivity
  double reduce_susceptibility;    // What percentage does it reduce susceptability
  double reduce_infectious_period; // What percentage does AV reduce infectious period
  double percent_symptomatics;     // Percentage of symptomatics recieving this drug
  double reduce_asymp_period;      // What percentage does it reduce the asymptomatic period
  double reduce_symp_period;       // What percentage does it reduce the symptomatic period
  double prob_symptoms;            // What is the probability of being symptomatic
  double efficacy;                 // The effectiveness of the AV (resistance)
  double* av_course_start_day;     // Probabilistic AV start
  int max_av_course_start_day;     // Maximum start day

  //Logistics
  int initial_stock;               // Amount of AV at day 0
  int stock;                       // Amount of AV currently available
  int additional_per_day;          // Amount of AV added to the system per day
  int total_avail;                 // The total amount available to the simulation
  int reserve;                     // Amount of AV left unused
  int start_day;                   // Day that AV is available to the system
  
  
  //Policy variables
  bool prophylaxis;                // Is this AV allowed for prophylaxis
  int percent_of_symptomatics;     // Percent of symptomatics that get this AV as treatment
  
  // For Statistics
  int given_out;               
  int ineff_given_out;
  
  //Policy
  Policy* policy;
  
  //To Do... Hospitalization based policy
};
#endif // _FRED_ANTIVIRAL_H

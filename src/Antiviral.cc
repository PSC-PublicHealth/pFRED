/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Antiviral.cc
//

#include <iomanip>

#include "Antiviral.h"
#include "AV_Health.h"
#include "Random.h"
#include "Person.h"
#include "Health.h"
#include "Infection.h"
#include "Disease.h"
#include "Global.h"

Antiviral::Antiviral(int _disease, int _course_length, double _reduce_infectivity,
                     double _reduce_susceptibility, double _reduce_asymp_period,
                     double _reduce_symp_period, double _prob_symptoms,
                     int _initial_stock, int _total_avail, int _additional_per_day,
                     double _efficacy, double* _av_course_start_day, 
                     int _max_av_course_start_day, int _start_day, bool _prophylaxis,
                     double _percent_symptomatics){
  disease                        = _disease;
  course_length                 = _course_length;
  reduce_infectivity            = _reduce_infectivity;
  reduce_susceptibility         = _reduce_susceptibility;
  reduce_asymp_period           = _reduce_asymp_period;
  reduce_symp_period            = _reduce_symp_period;
  prob_symptoms                 = _prob_symptoms;
  stock                         = _initial_stock;
  initial_stock                 = _initial_stock;
  reserve                       = _total_avail - _initial_stock;
  total_avail                   = _total_avail;
  additional_per_day            = _additional_per_day;
  efficacy                      = _efficacy;
  av_course_start_day           = _av_course_start_day;
  max_av_course_start_day       = _max_av_course_start_day;
  start_day                     = _start_day;
  prophylaxis                   = _prophylaxis;
  percent_symptomatics          = _percent_symptomatics;
}

int Antiviral::roll_will_have_symp() const {
  return (RANDOM() < prob_symptoms);
}

int Antiviral::roll_efficacy() const {
  return (RANDOM() < efficacy);
}

int Antiviral::roll_course_start_day() const {
  int days = 0;
  days = draw_from_distribution(max_av_course_start_day, av_course_start_day);
  return days;
}

void Antiviral::update(int day){
  if(day >= start_day) add_stock(additional_per_day);
}

void Antiviral::print() const {
  cout << "\tEffective for Disease \t\t"<< disease << "\n";
  cout << "\tCurrent Stock:\t\t\t"<<stock<< " out of "<< total_avail << "\n";
  cout << "\tWhat is left:\t\t\t"<< reserve << "\n";
  cout << "\tAdditional Per Day: \t\t"<< additional_per_day << "\n";
  //cout << "\tPercent Resistance\t\t"<<efficacy << "\n";
  cout << "\tReduces by:" << "\n";
  cout << "\t\tInfectivity:\t\t"<<reduce_infectivity*100.0 << "%\n";
  cout << "\t\tSusceptibility:\t\t"<<reduce_susceptibility*100.0 << "%\n";
  cout << "\t\tAymptomatic Period:\t" << reduce_asymp_period*100.0 << "%\n";
  cout << "\t\tSymptomatic Period:\t" << reduce_symp_period*100.0 << "%\n";
  cout << "\tNew Probability of symptoms:\t" << prob_symptoms << "\n";
  if(prophylaxis==1)
    cout <<"\tCan be given as prophylaxis" << "\n";
  if(percent_symptomatics != 0)
    cout << "\tGiven to percent symptomatics:\t" << percent_symptomatics*100.0 << "%\n";

  cout << "\n\tAV Course start day (max " << max_av_course_start_day << "):";
  for (int i = 0; i <= max_av_course_start_day; i++) {
    if((i%5)==0) cout << "\n\t\t";
    cout << setw(10) << setprecision(6) <<av_course_start_day[i] << " ";
  }
  cout << "\n";
}

void Antiviral::reset() { 
  stock = initial_stock;
  reserve = total_avail-initial_stock;
}

void Antiviral::print_stocks() const {
  cout << "Current: "<< stock << " Reserve: "<<reserve << " TAvail: "<< total_avail << "\n";
}

void Antiviral::report(int day) const {
  //STB - Need add a report utility
  cout << "\nNeed to write a report function";
}


int Antiviral::quality_control(int ndiseases) const {
  // Currently, this checks the parsing of the AVs, and it returns 1 if there is a problem
  if(disease < 0 || disease > ndiseases ) {
    cout << "\nAV disease invalid,cannot be higher than "<< ndiseases << "\n";
    return 1;
  }
  if(initial_stock < 0){
    cout <<"\nAV initial_stock invalid, cannot be lower than 0\n";
    return 1;
  }  
  if(efficacy>100 || efficacy < 0){
    cout << "\nAV Percent_Resistance invalid, must be between 0 and 100\n";
    return 1;
  }
  if(course_length < 0){
    cout << "\nAV Course Length invalid, must be higher than 0\n";
    return 1;
  }
  if(reduce_infectivity < 0 || reduce_infectivity > 1.00){
    cout << "\nAV reduce_infectivity invalid, must be between 0 and 1.0\n";
    return 1;
  }
  if(reduce_susceptibility < 0 || reduce_susceptibility > 1.00){
    cout << "\nAV reduce_susceptibility invalid, must be between 0 and 1.0\n";
    return 1;
  }
  if(reduce_infectious_period < 0 || reduce_infectious_period > 1.00){
    cout << "\nAV reduce_infectious_period invalid, must be between 0 and 1.0\n";
    return 1;
  }
  return 0;
}

void Antiviral::effect(Health *health, int cur_day, AV_Health* av_health){
  // We need to calculate the effect of the AV on all diseases it is applicable to
  int ndiseases = health->get_num_diseases();
  for (int is = 0; is < ndiseases; is++) {
    if(is == disease){ //Is this antiviral applicable to this disease
      // If this is the first day of AV Course
      if(cur_day == av_health->get_av_start_day()) {
        health->modify_susceptibility(is,1.0-reduce_susceptibility);
        // If you are already exposed, we need to modify your infection
        if((health->get_exposure_date(is) > -1) && (cur_day > health->get_exposure_date(is))){
          if(Debug > 3) cout << "reducing an already exposed person\n";
          health->modify_infectivity(is,1.0-reduce_infectivity);
	  if (!health->is_symptomatic() && cur_day < health->get_symptomatic_date(is)) {
	    // Can only have these effects if the agent is not symptomatic yet
	    health->modify_develops_symptoms(is,roll_will_have_symp(), cur_day);
	    health->modify_asymptomatic_period(is,1.0-reduce_asymp_period,cur_day);
	  }
	  if (health->is_symptomatic() && cur_day < health->get_recovered_date(is)) {
	    health->modify_symptomatic_period(is,1.0-reduce_symp_period,cur_day);
	  }
        }
      }
      // If today is the day you got exposed, prophilaxis
      if(cur_day == health->get_exposure_date(is)) { 
        if(Debug > 3) cout << "reducing agent on the day they are exposed\n";
        health->modify_infectivity(is,1.0-reduce_infectivity);
	if (!health->is_symptomatic() && cur_day < health->get_symptomatic_date(is)) {
	  // Can only have these effects if the agent is not symptomatic yet
	  health->modify_develops_symptoms(is, roll_will_have_symp(), cur_day);
	  health->modify_asymptomatic_period(is, 1.0-reduce_asymp_period, cur_day);
	}
	if (health->is_symptomatic() && cur_day < health->get_recovered_date(is)) {
	  health->modify_symptomatic_period(is,1.0-reduce_symp_period,cur_day);
	}
      }
      // If this is the last day of the course
      if(cur_day == av_health->get_av_end_day()) {
        if(Debug > 3) cout << "resetting agent to original state\n";
        health->modify_susceptibility(is,1.0/(1.0-reduce_susceptibility));
        if(cur_day >= health->get_exposure_date(is)) {
          health->modify_infectivity(is,1.0/(1.0-reduce_infectivity));
        }
      }  
    }
  }
}


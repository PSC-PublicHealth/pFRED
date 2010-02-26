/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Antiviral.cc
//

#include "Antiviral.h"
#include "Random.h"
#include "Person.h"
#include "Health.h"
#include "Infection.h"
#include "Strain.h"

Antiviral::Antiviral(int s, int cl, double ri, double rs, double reduce_asymp_per, double reduce_symp_per,
		     double prob_symp, int st, double eff, double* av_start, int max_av_start){
  strain = s;
  course_length = cl;
  reduce_infectivity = ri;
  reduce_susceptibility = rs;
  reduce_asymp_period = reduce_asymp_per;
  reduce_symp_period = reduce_symp_per;
  prob_symptoms = prob_symp;
  //percent_symptomatics = ps;
  stock = st;
  initial_stock = st;
  efficacy = eff;
  av_start_day = av_start;
  max_av_start_day = max_av_start;
}

int Antiviral::roll_will_have_symp(void) const {
  return (RANDOM() < prob_symptoms);
}

int Antiviral::roll_efficacy(void) const {
  return (RANDOM() < efficacy);
}

int Antiviral::roll_start_day(void) const {
 int days = 0;
 days = draw_from_distribution(max_av_start_day, av_start_day);
 return days;
}


void Antiviral::print(void){
  cout << "\n Effective for Strain \t\t"<< strain;
  //cout << "\n Percent of Symptomatics Recieving\t" << percent_symptomatics;
  cout << "\n Current Stock\t\t"<<stock<< " out of "<< initial_stock;
  cout << "\n Percent Resistance\t\t"<<efficacy;
  cout << "\n Course Length\t\t"<<course_length;
  cout << "\n Reduces:";
  cout << "\n\tInfectivity:\t"<<reduce_infectivity;
  cout << "\n\tSusceptibility\t"<<reduce_susceptibility;
  cout << "\n\tAymptomatic Period\t" << reduce_asymp_period;
  cout << "\n\tSymptomatic Period\t" << reduce_symp_period;
  cout << "\n\tProbability of symptoms:\t" << prob_symptoms;
  cout << "\n\tAV start day (max " << max_av_start_day << "): ";
  for (int i = 0; i <= max_av_start_day; i++) {
    cout << av_start_day[i] << " ";
  }
  cout << "\n";
}

void Antiviral::reset(void){
  stock = initial_stock;
}

void Antiviral::report(int day){
  //Don't know what to do yet
  cout << "\nNeed to write a report function";
}

int Antiviral::quality_control(int nstrains){
  // Currently, this checks the parsing of the AVs, and it returns 1 if there is a problem
  if(strain < 0 || strain > nstrains ) {
    cout << "\nAV strain invalid,cannot be higher than "<< nstrains << "\n";
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

  
  

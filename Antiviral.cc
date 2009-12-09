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

Antiviral::Antiviral(int s, int cl, double ri, double rs, double rip, 
		     int st, double eff){
  strain = s;
  course_length = cl;
  reduce_infectivity = ri;
  reduce_susceptibility = rs;
  reduce_infectious_period = rip;
  //percent_symptomatics = ps;
  stock = st;
  initial_stock = st;
  efficacy = eff;
}

//int Antiviral::roll_sympt(void){
//  return (RANDOM() < percent_symptomatics/100.0);
//}

int Antiviral::roll_efficacy(void){
  return (RANDOM() < efficacy);
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
  cout << "\n\tInfectious Period\t"<<reduce_infectious_period;
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

  
  

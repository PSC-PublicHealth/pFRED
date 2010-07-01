/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Antivirals.cc
//

#include <stdio.h>
#include <new>

using namespace std;
#include "Antivirals.h"
#include "Antiviral.h"
#include "Global.h"
#include "Params.h"
#include "Person.h"
#include "Health.h"
extern int Strains;

Antivirals::Antivirals(void){
  char s[80];
  int nav;
  
  sprintf(s,"number_antivirals");
  get_param((char *) "number_antivirals",&nav);
  
  for(int iav=0;iav<nav;iav++){
    int Strain, CorLength, InitSt, TotAvail, PerDay;
    double RedInf, RedSusc, RedASympPer, RedSympPer, ProbSymp, Eff, PerSympt;
    int StrtDay,Proph;
    bool isProph;
    // Check for deprecated params
    sprintf(s,"av_reduce_infectious_period[%d]",iav);
    if (does_param_exist(s)) {
      printf("***** Found deprecated parameter ***** %s\n"
	     "New parameters are av_reduce_asymptomatic_period and "
	     "av_reduce_symptomatic_period \n"
	     "Aborting\n", s);
      abort();
    }
    
    sprintf(s,"av_strain[%d]",iav);
    get_param(s,&Strain);
    
    sprintf(s,"av_initial_stock[%d]",iav);
    get_param(s,&InitSt);
    
    sprintf(s,"av_total_avail[%d]",iav);
    get_param(s,&TotAvail);
    
    sprintf(s,"av_additional_per_day[%d]",iav);
    get_param(s,&PerDay);
    
    sprintf(s,"av_percent_resistance[%d]",iav);
    get_param(s,&Eff);
    
    sprintf(s,"av_course_length[%d]",iav);
    get_param(s,&CorLength);
    
    sprintf(s,"av_reduce_infectivity[%d]",iav);
    get_param(s,&RedInf);
    
    sprintf(s,"av_reduce_susceptibility[%d]",iav);
    get_param(s,&RedSusc);
    
    sprintf(s,"av_reduce_symptomatic_period[%d]",iav);
    get_param(s, &RedSympPer);
    
    sprintf(s,"av_reduce_asymptomatic_period[%d]",iav);
    get_param(s, &RedASympPer);
    
    sprintf(s,"av_prob_symptoms[%d]",iav);
    get_param(s, &ProbSymp);
    
    sprintf(s, "av_start_day[%d]", iav);
    get_param(s, &StrtDay);
    
    sprintf(s, "av_prophylaxis[%d]", iav);
    get_param(s, &Proph);
    if(Proph == 1) isProph= true;
    else isProph = false;
    
    sprintf(s, "av_percent_symptomatics[%d]",iav);
    get_param(s, &PerSympt);
    
    sprintf(s, "av_course_start_day[%d]", iav);
    int n;
    get_param(s, &n);
    double* AVCourseSt= new double [n];
    int MaxAVCourseSt = get_param_vector(s, AVCourseSt) -1;
    
    AVs.push_back(new Antiviral(Strain, CorLength, RedInf, 
				RedSusc, RedASympPer, RedSympPer,
				ProbSymp, InitSt, TotAvail, PerDay, 
				Eff, AVCourseSt, MaxAVCourseSt,
				StrtDay, isProph, PerSympt) );
    
  }
  print();
  quality_control(Strains);
}

int  Antivirals::get_total_current_stock(void) const {
  int sum = 0;
  for(unsigned int i=0;i<AVs.size();i++) sum += AVs[i]->get_current_stock();
  return sum;
}

vector < Antiviral* >  Antivirals::find_applicable_AVs(int strain) const { 
  vector <Antiviral* > avs;
  for(unsigned int iav=0;iav< AVs.size();iav++){
    if(AVs[iav]->get_strain() == strain && AVs[iav]->get_current_stock() != 0){
      avs.push_back(AVs[iav]);
    }
  }
  return avs;
}

vector < Antiviral* >  Antivirals::prophylaxis_AVs(void) const {
  vector <Antiviral*> avs;
  for(unsigned int iav=0;iav< AVs.size();iav++){
    if(AVs[iav]->is_prophylaxis()){
      avs.push_back(AVs[iav]);
    }
  }
  return avs;
}

void  Antivirals::print(void) const {
  cout << "\n Antivirals Printout";
  cout << "\n There are "<< AVs.size() << " antivirals to choose from";
  for(unsigned int iav=0;iav<AVs.size(); iav++){
    AVs[iav]->print();
  }
  cout << "\n\n";
}

void  Antivirals::print_stocks(void) const {
  for(unsigned int iav = 0; iav < AVs.size(); iav++){
    cout <<"\n Antiviral #" << iav;
    AVs[iav]->print_stocks();
    cout <<"\n";
  }
}

void  Antivirals::quality_control(int nstrains) const {  
  for(unsigned int iav = 0;iav < AVs.size();iav++) {
    if (Verbose > 1) {
      AVs[iav]->print();
    }
    if(AVs[iav]->quality_control(nstrains)){
      cout << "\nHelp! AV# "<<iav << " failed Quality\n";
      exit(1);
    }
  }
}

void  Antivirals::update(int day){
  for(unsigned int iav =0;iav < AVs.size(); iav++)
    AVs[iav]->update(day);
}

void  Antivirals::report(int day) const {
  // STB - To Do
}

void  Antivirals::reset(void){
  for(unsigned int iav = 0; iav < AVs.size(); iav++)
    AVs[iav]->reset();
}


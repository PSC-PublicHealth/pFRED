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

int Antivirals::prophylaxis_start_date = -1;

Antivirals::Antivirals(void){
}

void Antivirals::setup(void){
  char s[80];
  int nav;
  
  sprintf(s,"number_antivirals");
  //get_param(s, &nav);
  get_param((char *) "number_antivirals",&nav);
  //AVs = new (nothrow) vector < Antiviral > [nav];
  //if(AVs == NULL){
  //  printf("Help! AVs vector allocation failure\n");
  //  abort();
  //}

 
  
  if(nav !=0){
    sprintf(s,"av_percent_symptomatics_given");
    get_param(s,&percent_symptomatics_given);

    sprintf(s,"av_prophylaxis_start_day");
    prophylaxis_start_date = -1;
    if (does_param_exist(s)) {
      get_param(s, &prophylaxis_start_date);
    }



    for(int iav=0;iav<nav;iav++){
      int sr, cl, st;
      double ri, rs, reduce_asymp_period, reduce_symp_period, reduce_prob_symptoms, eff;
        
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
      get_param(s,&sr);
      
      sprintf(s,"av_initial_stock[%d]",iav);
      get_param(s,&st);
      
      sprintf(s,"av_percent_resistance[%d]",iav);
      get_param(s,&eff);
      
      sprintf(s,"av_course_length[%d]",iav);
      get_param(s,&cl);

      sprintf(s,"av_reduce_infectivity[%d]",iav);
      get_param(s,&ri);
      
      sprintf(s,"av_reduce_susceptibility[%d]",iav);
      get_param(s,&rs);
      
      sprintf(s,"av_reduce_symptomatic_period[%d]",iav);
      get_param(s, &reduce_symp_period);

      sprintf(s,"av_reduce_asymptomatic_period[%d]",iav);
      get_param(s, &reduce_asymp_period);

      sprintf(s,"av_prob_symptoms[%d]",iav);
      get_param(s, &reduce_prob_symptoms);

      sprintf(s, "av_start_day[%d]", iav);
      int n;
      get_param(s, &n);
      double* av_start_day = new double [n];
      int max_av_start_day = get_param_vector(s, av_start_day) -1;
      
      AVs.push_back(new Antiviral(sr,cl,ri,rs, reduce_asymp_period, reduce_symp_period,
				  reduce_prob_symptoms, st, eff, av_start_day, max_av_start_day));
      AVs[AVs.size() - 1]->print();
    }
    quality_control(Strains);
  }
}

//void Antivirals::update(Person *pop, int pop_size, int day){

int Antivirals::give_which_AV(int strain){
  for(unsigned int iav=0;iav< AVs.size();iav++){
    if(AVs[iav]->get_strain() == strain && AVs[iav]->get_stock() != 0){
      return iav;
    }
  }
  return -1;
}

  // Based on the percentage of persons 
void Antivirals::print(void){
  cout << "\n Antivirals Printout";
  cout << "\n There are "<< AVs.size() << " antivirals to choose from";
  cout << "\n These will be given to \t\t"<< percent_symptomatics_given;
  for(unsigned int iav=0;iav<AVs.size(); iav++){
    AVs[iav]->print();
  }
  cout << "\n\n";
}

void Antivirals::reset(void){
  for(unsigned int iav = 0; iav < AVs.size();iav++)
    AVs[iav]->reset();
}

void Antivirals::quality_control(int nstrains){
  
  if(percent_symptomatics_given < 0 || percent_symptomatics_given > 100){
    cout << "\nHelp! AVs percent symptomatics_given is wrong\n";
    exit (1);
  }
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


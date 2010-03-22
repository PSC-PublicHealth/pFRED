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
#include "AV_Status.h"
#include "Random.h"
#include "Person.h"
#include "Health.h"
#include "Infection.h"
#include "Strain.h"

Antiviral::Antiviral(int Strain, int CorLength, double RedInf, 
		     double RedSuc, double RedASympPer, double RedSympPer,
		     double ProbSymp, int InitSt, int TotAvail, int PerDay, double Eff, 
		     double* AVCourseSt, int MaxAVCourseSt,
		     int StrtDay, int Proph, double PerSympt){
  strain = Strain;
  course_length                 = CorLength;
  reduce_infectivity            = RedInf;
  reduce_susceptibility         = RedSuc;
  reduce_asymp_period           = RedASympPer;
  reduce_symp_period            = RedSympPer;
  prob_symptoms                 = ProbSymp;
  stock                         = InitSt;
  initial_stock                 = InitSt;
  reserve                       = TotAvail-InitSt;
  total_avail                   = TotAvail;
  additional_per_day            = PerDay;
  efficacy                      = Eff;
  av_course_start_day           = AVCourseSt;
  max_av_course_start_day       = MaxAVCourseSt;
  start_day                     = StrtDay;
  prophylaxis                   = Proph;
  percent_symptomatics          = PerSympt;
}

int Antiviral::roll_will_have_symp(void) const {
  return (RANDOM() < prob_symptoms);
}

int Antiviral::roll_efficacy(void) const {
  return (RANDOM() < efficacy);
}

int Antiviral::roll_course_start_day(void) const {
 int days = 0;
 days = draw_from_distribution(max_av_course_start_day, av_course_start_day);
 return days;
}

void Antiviral::print(void){
  cout << "\n Effective for Strain \t\t"<< strain;
  //cout << "\n Percent of Symptomatics Recieving\t" << percent_symptomatics;
  cout << "\n Current Stock\t\t\t"<<stock<< " out of "<< total_avail;
  cout << "\n What is left =\t\t\t"<< reserve;
  cout << "\n Additional Per Day = \t\t"<< additional_per_day;
  cout << "\n Percent Resistance\t\t"<<efficacy;
  cout << "\n Reduces:";
  cout << "\n\tInfectivity:\t\t\t"<<reduce_infectivity;
  cout << "\n\tSusceptibility\t\t\t"<<reduce_susceptibility;
  cout << "\n\tAymptomatic Period\t\t" << reduce_asymp_period;
  cout << "\n\tSymptomatic Period\t\t" << reduce_symp_period;
  cout << "\n\tProbability of symptoms:\t" << prob_symptoms;
  if(prophylaxis==1)
    cout <<"\n\tCan be given as prophylaxis";
  if(percent_symptomatics != 0)
    cout << "\n\tGiven to percent symptomatics:\t" << percent_symptomatics;
  
  
  cout << "\n\tAV Course start day (max " << max_av_course_start_day << "):";
  for (int i = 0; i <= max_av_course_start_day; i++) {
    if((i%5)==0) cout << "\n\t\t\t";
    cout << av_course_start_day[i] << " ";
  }
  cout << "\n";
}

void Antiviral::print_stocks(void){
  cout << "\n Current: "<< stock << " Reserve: "<<reserve << " TAvail: "<< total_avail;
}

void Antiviral::reset(void){
  stock = initial_stock;
  reserve = total_avail-initial_stock;
}

void Antiviral::report(int day){
  //Don't know what to do yet
  cout << "\nNeed to write a report function";
}

void Antiviral::update(int day){
  if(day >= start_day) add_stock(additional_per_day);
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

void Antiviral::effect(Health *h, int cur_day, AV_Status* av_stats){
  // We need to calculate the effect of the AV on all strains it is applicable to
  int nstrains = h->get_num_strains();
  for(int is=0;is<nstrains;is++){
    if(is == strain){ //Is this antiviral applicable to this strain
      // If this is the first day of AV Course
      //cout <<"\nIn effect is = " << is << " av start day = "<< av_stats->get_av_start_day() << " cur_day = "<<cur_day;
      if(cur_day == av_stats->get_av_start_day()){
	h->modify_susceptibility(is,reduce_susceptibility);
	// If you are already exposed, we need to modify your infection
	if((h->get_exposure_date(is) > -1) && (cur_day > h->get_exposure_date(is))){
	  cout << "\nreducing an already exposed person";
	  h->modify_infectivity(is,reduce_infectivity);
	  h->modify_develops_symptoms(is,roll_will_have_symp(), cur_day);
	  h->modify_asymptomatic_period(is,reduce_asymp_period,cur_day);
	  h->modify_symptomatic_period(is,reduce_symp_period,cur_day);
	}
      }
      // If today is the day you got exposed, prophilaxis
      if(cur_day == h->get_exposure_date(is)){ 
	cout << "\nreducing agent on the day they are exposed";
	h->modify_infectivity(is,reduce_infectivity);
	h->modify_develops_symptoms(is,roll_will_have_symp(),cur_day);
	h->modify_asymptomatic_period(is,reduce_asymp_period,cur_day);
	h->modify_symptomatic_period(is,reduce_symp_period,cur_day);
      }
      // If this is the last day of the course
      if(cur_day == av_stats->get_av_end_day()){
	cout << "\nresetting agent to original state";
	h->modify_susceptibility(is,1.0/reduce_susceptibility);
	if(cur_day >= h->get_exposure_date(is)){
	  h->modify_infectivity(is,1.0/reduce_infectivity);
	}
      }  
    }
  }
  cout <<"\n";
}

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Status.cc
//


#include <stdio.h>
#include <assert.h>
#include <iostream>
#include "Random.h"
#include "AV_Status.h"
#include "Antiviral.h"
#include "Health.h"
#include "Strain.h"
#include "Person.h"
#include "Global.h"

using namespace std;

AV_Status::AV_Status(int day, Antiviral* av, Health* h){
  AV = av;
  strain = av->get_strain();
  av_day = day +1;
  health = h;
  av_end_day = -1;
  double efficacy = AV->get_efficacy();
  av_end_day = av_day + av->get_course_length();
} 

void AV_Status::print(void){
  // Need to figure out what to write here
  cout << "\nAV_Status";
}

void AV_Status::printTrace(void){
  fprintf(Tracefp," %2d %2d %2d",av_day,strain,is_effective());
}

void AV_Status::update(int day){
  if(day <= av_end_day){
    if(health->get_infection(0)!=NULL){
      cout << "\nBefore\n"; 
      health->get_infection(0)->print();
    }
  }
  AV->effect(health,day,this);
  if(day <= av_end_day){
    if(health->get_infection(0)!=NULL){
      cout << "\nAfter\n";
      health->get_infection(0)->print();
    }
  }
}

  
    
    

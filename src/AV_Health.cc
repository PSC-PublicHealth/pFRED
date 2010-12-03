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
#include "AV_Health.h"
#include "Antiviral.h"
#include "Health.h"
#include "Disease.h"
#include "Person.h"
#include "Global.h"

using namespace std;

AV_Health::AV_Health(int _av_day, Antiviral* _AV, Health* _health){
  AV              = _AV;
  disease          = AV->get_disease();
  av_day          = _av_day +1;
  health          = _health;
  av_end_day      = -1;
  av_end_day      = av_day + AV->get_course_length();
} 

void AV_Health::print() const {
  // Need to figure out what to write here
  cout << "\nAV_Status";
}

void AV_Health::printTrace() const {
  fprintf(Tracefp," %2d %2d %2d",av_day,disease,is_effective());
}

void AV_Health::update(int day){
  if(day <= av_end_day){
    if(health->get_infection(0)!=NULL){
      if(Debug > 3) {
        cout << "\nBefore\n"; 
        health->get_infection(0)->print();
      }
    }
    else{
      if(Debug > 3)
	cout << "\nBefore: Suceptibility "<< health->get_susceptibility(0) << "\n";
    }
  }
  AV->effect(health,day,this);
  if(day <= av_end_day){
    if(Debug > 3) {
      if(health->get_infection(0)!=NULL){
        cout << "\nAfter\n";
        health->get_infection(0)->print();
      }
      else{
	if(Debug > 3)
	  cout << "\nAfter: Suceptibility "<< health->get_susceptibility(0) << "\n";
      }
    }
  }
}





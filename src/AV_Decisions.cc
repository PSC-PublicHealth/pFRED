/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Policies.cpp
//

#include "AV_Decisions.h"
#include "AV_Policies.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "Antivirals.h"
#include "Antiviral.h"
#include "Random.h"
#include "Person.h"
#include "Health.h"
#include <iostream>

AV_Decision_Allow_Only_One::AV_Decision_Allow_Only_One(Policy * p):
  Decision(p){
  Name = "AV Decision Allow Only One AV per Person";
  Type = "Y/N";
  policy = p;
}

int AV_Decision_Allow_Only_One::evaluate(void){
  Person *p = policy->get_manager()->get_current_person();
  if(p->get_health()->get_number_av_taken() == 0) return 0;
  else return -1;
}

AV_Decision_Give_to_Sympt::AV_Decision_Give_to_Sympt(Policy *p):
  Decision(p){
  Name = "AV Decision to give to a percentage of symptomatics";
  Type = "Y/N";
  policy = p;
}

int AV_Decision_Give_to_Sympt::evaluate(void){
  Person *p = policy->get_manager()->get_current_person();
  int strain = policy->get_manager()->get_current_strain();
  AV_Manager *avm = dynamic_cast < AV_Manager* > ( policy->get_manager() );
  Antiviral *av = avm->get_current_av();
  int percentage = av->get_percent_symptomatics();
  if(p->get_health()->get_strain_status(strain) == 'I'){
    p->get_health()->flip_checked_for_av(strain);
    double r = RANDOM()*100.;
    if( r < percentage ) return 0;
  }
  return -1;
}

AV_Decision_Begin_AV_On_Day::AV_Decision_Begin_AV_On_Day(Policy *p):
  Decision(p){
  Name = "AV Decision to Begin disseminating AVs on a certain day";
  Type = "Y/N";
  policy = p;
}

int AV_Decision_Begin_AV_On_Day::evaluate(void){
  AV_Manager *avm = dynamic_cast < AV_Manager* > ( policy->get_manager() );
  Antiviral* av = avm->get_current_av();
  int start_day = av->get_start_day();
  if(avm->get_current_day() >=start_day) { return 0;}
  return -1;
}

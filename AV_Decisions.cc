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

AV_Decision_Give_One_Chance::AV_Decision_Give_One_Chance(Policy *p):
  Decision(p){
  Name = "AV Decision Give One chance to get an AV per strain";
  Type = "Y/N";
  policy = p;
}

int AV_Decision_Give_One_Chance::evaluate(void){
  Person *p = policy->get_manager()->get_current_person();
  int strain = policy->get_manager()->get_current_strain();
  //cout << "\nGive One strain "<<strain << " checked? " << p->get_health()->get_checked_for_av(strain);
  if(p->get_health()->get_checked_for_av(strain) == 0){
    p->get_health()->flip_checked_for_av(strain);
    return 0;
  }
  return -1;
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
  int  percentage = avm->get_antivirals()->get_percent_symptomatics_given();
  cout <<"\n dec strain "<< strain << " percentage " << percentage << " status " << p->get_health()->get_strain_status(strain);
  if(p->get_health()->get_strain_status(strain) == 'I'){
    double r = RANDOM()*100.;
    if( r < percentage ) return 0;
  }
  return -1;
}

AV_Decision_Which_AV_is_Available::AV_Decision_Which_AV_is_Available(Policy *p):
  Decision(p){
  Name = "AV Decision which AVs are available";
  Type = "WhichOne";
  policy = p;
}

int AV_Decision_Which_AV_is_Available::evaluate(void){
  Person *p = policy->get_manager()->get_current_person();
  int strain = policy->get_manager()->get_current_strain();
  AV_Manager *avm = dynamic_cast < AV_Manager* > ( policy->get_manager() );
  int av_to_give = avm->get_antivirals()->give_which_AV(strain);
  return av_to_give;
}


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Manager.cpp
//

#include "Global.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "Policy.h"
#include "AV_Policies.h"
#include "Population.h"
#include "Antivirals.h"
#include "Antiviral.h"
#include "Params.h"
#include "Person.h"

AV_Manager::AV_Manager(Population *_pop):
  Manager(_pop){
  pop = _pop;
  
  char s[80];
  int nav;
  sprintf(s,"number_antivirals");
  get_param((char *) "number_antivirals",&nav);
 
  do_av=0;
  if(nav > 0){
    do_av = 1;
    av_package = new Antivirals();
 
    // Gather relavent Input Parameters
    sprintf(s,"av_overall_start_day");
    overall_start_day = 0;
    if(does_param_exist(s))
      get_param(s,&overall_start_day);

    // Need to fill the AV_Manager Policies
    policies.push_back(new AV_Policy_Distribute_To_Symptomatics(this)); 
    policies.push_back(new AV_Policy_Distribute_To_Everyone(this));
    
    // Need to run through the Antivirals and give them the appropriate policy
    set_policies();
  }
  else{
    overall_start_day = -1;
  }
}

void AV_Manager::update(int day){
  if(do_av==1){
    current_day = day;
    av_package->update(day);
    //   if(Debug > 1){
      av_package->print_stocks();
      //}
  }
}

void AV_Manager::reset(void){
  current_day = -1;
  current_person = NULL;
  current_strain = -1;
  if(do_av==1){
    av_package->reset();
  }
}

void AV_Manager::print(void){
  if(do_av == 1)
    av_package->print();
}			

void AV_Manager::set_policies(void){
  vector < Antiviral* > avs = av_package->get_AV_vector();
  for(unsigned int iav = 0;iav<avs.size();iav++){
    if(avs[iav]->is_prophylaxis()){
      avs[iav]->set_policy(policies[AV_POLICY_GIVE_EVERYONE]);
    }
    else{ 
      avs[iav]->set_policy(policies[AV_POLICY_PERCENT_SYMPT]);
    }
  }
}

void AV_Manager::disseminate(int day){
  // There is no queue, only the whole population
  if(do_av==0) return;
  Person* people = pop->get_pop();
  int npeople = pop->get_pop_size();
  current_day = day;
  // The av_package are in a priority based order, so lets loop over the av_package first
  vector < Antiviral* > avs = av_package->get_AV_vector();
  for(unsigned int iav = 0; iav<avs.size(); iav++){
    Antiviral * av = avs[iav];
    if(av->get_current_stock() > 0){
      // Each AV has its one policy
      Policy *p = av->get_policy();
      
      current_av = av;
      current_strain = av->get_strain();
      
      for(int ip=0;ip<npeople;ip++){
	if(av->get_current_stock()== 0) break;
	current_person = &people[ip];
	// Should the person get an av
	int yeahorney = p->choose();
	if(yeahorney == 0){
	  if(Debug > 2) cout << "Giving Antiviral for strain " << av->get_strain() << " to " <<ip << "\n";
	  av->remove_stock(1);
	  current_person->get_health()->take(av,current_day);
	}
      }
    }
  }
}
        
	  

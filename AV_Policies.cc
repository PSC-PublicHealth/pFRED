/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Policies.h
//

#include <iostream>
#include <vector>
#include "Decision.h"
#include "AV_Policies.h"
#include "AV_Decisions.h"
#include "Policy.h"
#include "Manager.h"
#include "AV_Manager.h"

AV_Policy_Distribute_To_Symptomatics::AV_Policy_Distribute_To_Symptomatics(AV_Manager* avm):
  Policy(dynamic_cast <Manager *> (avm)){
  Name = "Distribute AVs to Symptomatics";
  //av_manager = avm;
  
  // Need to add the policies in the decisio
  Decision_list.push_back(new AV_Decision_Give_One_Chance(this));
  Decision_list.push_back(new AV_Decision_Give_to_Sympt(this));
  Decision_list.push_back(new AV_Decision_Allow_Only_One(this));
  Decision_list.push_back(new AV_Decision_Which_AV_is_Available(this));
}

int AV_Policy_Distribute_To_Symptomatics::choose(void) {
  int result=-1;
  for(unsigned int i=0; i < Decision_list.size(); i++){
    
    int new_result = Decision_list[i]->evaluate();
    if(new_result == -1) return -1;
    else if(new_result > result) result = new_result;
    cout <<"\nResult for decision "<<i<< " is "<< result;
  }
  return result;
}   

AV_Policy_Distribute_To_Everyone::AV_Policy_Distribute_To_Everyone(AV_Manager* avm):
  Policy(dynamic_cast <Manager *> (avm)){
  Name = "Distribute AVs to Symptomatics";
  //av_manager = avm;
  
  // Need to add the policies in the decisio
  Decision_list.push_back(new AV_Decision_Give_One_Chance(this));
  Decision_list.push_back(new AV_Decision_Allow_Only_One(this));
  Decision_list.push_back(new AV_Decision_Which_AV_is_Available(this));
}

int AV_Policy_Distribute_To_Everyone::choose(void){
  int result=-1;
  for(unsigned int i=0; i < Decision_list.size(); i++){
    
    int new_result = Decision_list[i]->evaluate();
    if(new_result > result) result = new_result;
    cout <<"\nResult for decision "<<i<< " is "<< result;
    if(result == -1) 
      return -1;
  }
  return result;
}

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Vaccine_Priority_Policies.cc
//

#include <iostream>
#include <vector>

#include "Decision.h"
#include "Vaccine_Priority_Policies.h"
#include "Vaccine_Priority_Decisions.h"
#include "Policy.h"
#include "Manager.h"
#include "Vaccine_Manager.h"

Vaccine_Priority_Policy_No_Priority::Vaccine_Priority_Policy_No_Priority(Vaccine_Manager *vcm):
  Policy(dynamic_cast <Manager *> (vcm)){
  
  Name = "Vaccine Priority Policy - No Priority";
  
  decision_list.push_back(new Vaccine_Priority_Decision_No_Priority(this));
}

//This is generic, do not need this to be explicit.  NEED TO FIX
int Vaccine_Priority_Policy_No_Priority:: choose(Person* person,int strain,int day){
  int result=-1;
  for(unsigned int i=0; i < decision_list.size(); i++){
    
    int new_result = decision_list[i]->evaluate(person,strain,day);
    if(new_result == -1) return -1;
    else if(new_result > result) result = new_result;
    //cout <<"\nResult for decision "<<i<< " is "<< result;
  }
  return result;
}  

Vaccine_Priority_Policy_Specific_Age::Vaccine_Priority_Policy_Specific_Age(Vaccine_Manager *vcm):
  Policy(dynamic_cast <Manager *> (vcm)){
  
  Name = "Vaccine Priority Policy - Sepcific Age Group";
  decision_list.push_back(new Vaccine_Priority_Decision_Specific_Age(this));
}

//This is generic, do not need this to be explicit.  NEED TO FIX
int Vaccine_Priority_Policy_Specific_Age:: choose(Person* person, int strain, int day){
  int result = -1;
  for(unsigned int i=0; i < decision_list.size(); i++){
    int new_result = decision_list[i]->evaluate(person,strain,day);
    if(new_result == -1) return -1;
    else if(new_result > result) result = new_result;
    //cout <<"\nResult for decision "<<i<< " is "<< result;
  }
  return result;
}  

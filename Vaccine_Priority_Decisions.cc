/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Vaccine_Priority_Policies.cpp
//

#include "Vaccine_Priority_Decisions.h"
#include "Vaccine_Priority_Policies.h"
#include "Manager.h"
#include "Vaccine_Manager.h"
#include "Vaccines.h"
#include "Random.h"
#include "Person.h"
#include "Health.h"
#include <iostream>


Vaccine_Priority_Decision_Specific_Age::Vaccine_Priority_Decision_Specific_Age(Policy *p):
  Decision(p){
  Name = "Vaccine Priority Decision Specific Age";
  Type = "Y/N";
  policy = p;
}

int Vaccine_Priority_Decision_Specific_Age::evaluate(void){
  Person *p = policy->get_manager()->get_current_person();
  Vaccine_Manager* vcm = dynamic_cast < Vaccine_Manager* >(policy->get_manager());
  int low_age = vcm->get_vaccine_priority_age_low();
  int high_age = vcm->get_vaccine_priority_age_high();
  
  if(p->get_age() >= low_age && p->get_age() <= high_age){
    return 1;
  }
  return -1;
}

Vaccine_Priority_Decision_No_Priority::Vaccine_Priority_Decision_No_Priority(Policy *p):
  Decision(p){
  Name = "Vaccine Priority Decision No Priority";
  Type = "Y/N";
  policy=p;
}

int Vaccine_Priority_Decision_No_Priority::evaluate(void){
  //Basically, anyone is accepted into the normal queue
  return -1;
}


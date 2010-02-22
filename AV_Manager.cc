/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Manager.cpp
//

#include "Manager.h"
#include "AV_Manager.h"
#include "Policy.h"
#include "AV_Policies.h"
#include "Population.h"
#include "Antivirals.h"
#include "Params.h"

AV_Manager::AV_Manager(Population *P, Antivirals *avs):
  Manager(P){
  Pop = P;
  AVs = avs;
  // Need to fill the AV_Manager Policies
  Policies.push_back(new AV_Policy_Distribute_To_Symptomatics(this)); 
};

int AV_Manager::do_av(void){
  return AVs->do_av();
}

// int AV_Manager::poll_manager(Person *p, int strain){
//   current_person = p;
//   current_strain = strain;
//   int result = Policies[current_policy]->choose();
//   return result;
// }

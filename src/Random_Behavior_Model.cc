/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Random_Behavior_Model.cc
//

#include "Random_Behavior_Model.h"
#include "Person.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"


#include <stdio.h>
#include <iostream>

using namespace std;

int  Random_Behavior_Model_parameters_set = 0;

Random_Behavior_Model::Random_Behavior_Model(Person *p) {
  self = p;
  // printf("new Random Behavior Model\n"); fflush(stdout);

  // get parameters (one time only)
  //if (Random_Behavior_Model_parameters_set == 0) {
  //
  //  Random_Behavior_Model_parameters_set = 1;
  //}
  //  cout << "Prob of accept vaccine Dose = "<< Prob_accept_vaccine_dose << "\n";
}



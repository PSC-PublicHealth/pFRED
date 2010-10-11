/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Random_Cognitive_Model.cc
//

#include "Random_Cognitive_Model.h"
#include "Person.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"


#include <stdio.h>
#include <iostream>

using namespace std;

int  Random_Cognitive_Model_parameters_set = 0;

Random_Cognitive_Model::Random_Cognitive_Model(Person *p) {
  self = p;
  // printf("new Random Cognitive Model\n"); fflush(stdout);

  // get parameters (one time only)
  //if (Random_Cognitive_Model_parameters_set == 0) {
  //
  //  Random_Cognitive_Model_parameters_set = 1;
  //}
  //  cout << "Prob of accept vaccine Dose = "<< Prob_accept_vaccine_dose << "\n";
}



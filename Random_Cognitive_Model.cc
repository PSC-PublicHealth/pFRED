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

int  Random_Cognitive_Model_parameters_set = 0;
double Prob_accept_vaccine;

Random_Cognitive_Model::Random_Cognitive_Model(Person *p) {
  self = p;
  // printf("new Random Cognitive Model\n"); fflush(stdout);

  // get parameters (one time only)
  if (Random_Cognitive_Model_parameters_set == 0) {
    get_param((char *) "prob_accept_vaccine", &Prob_accept_vaccine);
    Random_Cognitive_Model_parameters_set = 1;
  }
}

bool Random_Cognitive_Model::will_accept_vaccine(int strain) {
  return RANDOM() < Prob_accept_vaccine;
}


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
double Prob_stay_home_if_sick;
double Prob_accept_vaccine;
double Prob_keep_kids_home;
double Prob_avoid_crowds;
double Prob_wear_face_mask;

Random_Cognitive_Model::Random_Cognitive_Model(Person *p) {
  self = p;
  
  // printf("new Random Cognitive Model\n"); fflush(stdout);

  // get parameters (one time only)
  if (Random_Cognitive_Model_parameters_set) return;
  get_param((char *) "prob_stay_home_if_sick", &Prob_stay_home_if_sick);
  get_param((char *) "prob_accept_vaccine", &Prob_accept_vaccine);
  get_param((char *) "prob_keep_kids_home", &Prob_keep_kids_home);
  get_param((char *) "prob_avoid_crowds", &Prob_avoid_crowds);
  get_param((char *) "prob_wear_face_mask", &Prob_wear_face_mask);
  Random_Cognitive_Model_parameters_set = 1;
}

bool Random_Cognitive_Model::will_stay_home_if_sick() {
  return RANDOM() < Prob_stay_home_if_sick;
}

bool Random_Cognitive_Model::will_accept_vaccine() {
  return RANDOM() < Prob_accept_vaccine;
}

bool Random_Cognitive_Model::will_avoid_crowds() {
  return RANDOM() < Prob_avoid_crowds;
}

bool Random_Cognitive_Model::will_keep_kids_home() {
  return RANDOM() < Prob_keep_kids_home;
}

bool Random_Cognitive_Model::will_wear_face_mask() {
  return RANDOM() < Prob_wear_face_mask;
}



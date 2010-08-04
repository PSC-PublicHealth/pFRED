/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Random_Cognitive_model.h
//

#ifndef _FRED_RANDOM_COGNITIVE_MODEL_H
#define _FRED_RANDOM_COGNITIVE_MODEL_H

#include "Cognitive_Model.h"

class Person;

extern int Random_Cognitive_Model_parameters_set;
extern double Prob_stay_home_if_sick;
extern double Prob_accept_vaccine;
extern double Prob_keep_kids_home;
extern double Prob_avoid_crowds;
extern double Prob_wear_face_mask;

class Random_Cognitive_Model : public Cognitive_Model {
 public:
  Random_Cognitive_Model(Person *p);
  void reset() {};
  void update(int day) {};
  bool will_stay_home_if_sick();
  bool will_accept_vaccine();
  bool will_avoid_crowds();
  bool will_keep_kids_home();
  bool will_wear_face_mask();
 private:
  Person * self;
 protected:
  ~Random_Cognitive_Model() {}
};

#endif // _FRED_RANDOM_COGNITIVE_MODEL_H


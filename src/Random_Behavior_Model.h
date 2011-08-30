/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Random_Behavior_model.h
//

#ifndef _FRED_RANDOM_COGNITIVE_MODEL_H
#define _FRED_RANDOM_COGNITIVE_MODEL_H

#include "Behavior_Model.h"

#include "Global.h"
#include "Random.h"

class Person;

class Random_Behavior_Model : public Behavior_Model {
 public:
  Random_Behavior_Model(Person *p);
  void update(int day) {
    accept_vaccine = (RANDOM() < Global::Prob_accept_vaccine);
    accept_vaccine_dose = (RANDOM() < Global::Prob_accept_vaccine_dose);
  }
  bool will_accept_vaccine(int disease)              { return accept_vaccine; }
  bool will_accept_another_vaccine_dose(int disease) { return accept_vaccine_dose; }
  bool is_staying_home(int day);

 private:
  Person* self;
  bool accept_vaccine;
  bool accept_vaccine_dose;              // If there is another dose, will someone take it
  bool staying_home;
  bool have_decided;

  // static values shared by all
  static double prob_stay_home;
  static bool initialized;

 protected:
  ~Random_Behavior_Model() {}
};

#endif // _FRED_RANDOM_COGNITIVE_MODEL_H


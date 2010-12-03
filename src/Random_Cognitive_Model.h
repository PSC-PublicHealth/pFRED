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

#include "Global.h"
#include "Random.h"

class Person;

//extern double Prob_accept_vaccine;

class Random_Cognitive_Model : public Cognitive_Model {
 public:
  Random_Cognitive_Model(Person *p);
  void reset() {};
  void update(int day) {
    accept_vaccine = (RANDOM() < Prob_accept_vaccine);
    accept_vaccine_dose = (RANDOM() < Prob_accept_vaccine_dose);
  }
  bool will_accept_vaccine(int disease)              { return accept_vaccine; }
  bool will_accept_another_vaccine_dose(int disease) { return accept_vaccine_dose; }
 private:
  Person* self;
  bool accept_vaccine;
  bool accept_vaccine_dose;              // If there is another dose, will someone take it
  //  double Prob_accept_vaccine_dose;      // Probability used when someone is faced with taking another vaccine

 protected:
  ~Random_Cognitive_Model() {}
};

#endif // _FRED_RANDOM_COGNITIVE_MODEL_H


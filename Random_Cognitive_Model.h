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

class Random_Cognitive_Model : public Cognitive_Model {
 public:
  Random_Cognitive_Model(Person *p);
  void reset() {};
  void update(int day) {};
  bool will_accept_vaccine(int strain);
 private:
  Person * self;
 protected:
  ~Random_Cognitive_Model() {}
};


extern double Prob_accept_vaccine;

inline bool Random_Cognitive_Model::will_accept_vaccine(int strain) {
  return (RANDOM() < Prob_accept_vaccine);
}

#endif // _FRED_RANDOM_COGNITIVE_MODEL_H


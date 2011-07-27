/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.h
//

#ifndef _FRED_BEHAVIOR_H
#define _FRED_BEHAVIOR_H

#include <vector>
#include "Global.h"
#include "Behavior_Model.h"

class Person;
class Disease;
class Transmission;

extern int V_count;

class Behavior {

public:
  Behavior(Person *p);
  ~Behavior() {}
  void update(int day) {
    model->update(day);
    V_count += will_accept_vaccine(0);
  }
  bool will_accept_vaccine(int disease) {
    return model->will_accept_vaccine(disease);
  }
  bool will_accept_another_vaccine_dose(int disease) {
    return model->will_accept_another_vaccine_dose(disease);
  }
  bool will_keep_kids_home() {
    return false;
  }
  bool acceptance_of_vaccine() {
    return will_accept_vaccine(0);
  }
  bool acceptance_of_another_vaccine_dose() {
    return will_accept_another_vaccine_dose(0);
  }
  void getInfected(Disease *disease, Transmission *transmission) {};
private:
  void get_parameters();
  Behavior_Model* model;       // behavior model does all the real work
};

#endif // _FRED_BEHAVIOR_H


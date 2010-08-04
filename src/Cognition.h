/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cognition.h
//

#ifndef _FRED_COGNITION_H
#define _FRED_COGNITION_H

#include "Cognitive_model.h"

class Person;

class Cognition {
 public:
  Cognition(Person *p);
  ~Cognition() {}
  void reset() { model->reset(); }
  void update(int day) { model->update(day); }
  bool will_stay_home_if_sick() { return model->will_stay_home_if_sick(); }
  bool will_accept_vaccine() { return model->will_accept_vaccine(); }
  bool will_avoid_crowds() { return model->will_avoid_crowds(); }
  bool will_keep_kids_home() { return model->will_keep_kids_home(); }
  bool will_wear_face_mask() { return model->will_wear_face_mask(); }
 private:
  void get_parameters();
  Cognitive_Model * model;	     // cognitive model does all the real work
};

#endif // _FRED_COGNITION_H


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
  bool will_accept_vaccine(int strain) { return model->will_accept_vaccine(strain); }
  bool will_keep_kids_home() { return false; }
 private:
  void get_parameters();
  Cognitive_Model * model;	     // cognitive model does all the real work
};

#endif // _FRED_COGNITION_H


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

#include "Global.h"
#include "Cognitive_Model.h"

class Person;
extern int V_count;

class Cognition {
 public:
  Cognition(Person *p);
  ~Cognition() {}
  void reset() { model->reset(); }
  void update(int day) { model->update(day); V_count += will_accept_vaccine(0); }
  bool will_accept_vaccine(int disease) { return model->will_accept_vaccine(disease); }
  bool will_keep_kids_home() { return false; }
 private:
  void get_parameters();
  Cognitive_Model * model;	     // cognitive model does all the real work
};

#endif // _FRED_COGNITION_H


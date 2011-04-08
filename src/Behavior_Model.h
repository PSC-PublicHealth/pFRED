/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior_model.h
//

#ifndef _FRED_COGNITIVE_MODEL_H
#define _FRED_COGNITIVE_MODEL_H

class Behavior_Model {
 public:
  Behavior_Model() {}
  ~Behavior_Model() {}
  virtual void reset() = 0;
  virtual void update(int day) = 0;
  virtual bool will_accept_vaccine(int disease) = 0;
  virtual bool will_accept_another_vaccine_dose(int disease) = 0;
};

#endif // _FRED_COGNITIVE_MODEL_H


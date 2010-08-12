/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cognitive_model.h
//

#ifndef _FRED_COGNITIVE_MODEL_H
#define _FRED_COGNITIVE_MODEL_H

class Cognitive_Model {
 public:
  Cognitive_Model() {}
  ~Cognitive_Model() {}
  virtual void reset() = 0;
  virtual void update(int day) = 0;
  virtual bool will_accept_vaccine(int strain) = 0;
};

#endif // _FRED_COGNITIVE_MODEL_H


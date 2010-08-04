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

#define ACTIONS 5

#define STAY_HOME_IF_SICK 0
// Note: may need separate decision for each strain
#define ACCEPT_VACCINE 1
#define KEEP_KIDS_HOME 2
#define AVOID_CROWDS 3
#define WEAR_FACE_MASK 4

class Cognitive_Model {
 public:
  Cognitive_Model() {}
  ~Cognitive_Model() {}
  virtual void reset() = 0;
  virtual void update(int day) = 0;
  virtual bool will_stay_home_if_sick() = 0;
  virtual bool will_accept_vaccine() = 0;
  virtual bool will_avoid_crowds() = 0;
  virtual bool will_keep_kids_home() = 0;
  virtual bool will_wear_face_mask() = 0;
};

#endif // _FRED_COGNITIVE_MODEL_H


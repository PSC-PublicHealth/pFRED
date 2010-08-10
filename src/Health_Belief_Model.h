/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health_Belief_Model.h
//

#ifndef _FRED_HEALTH_BELIEF_MODEL_H
#define _FRED_HEALTH_BELIEF_MODEL_H

#include "Cognitive_Model.h"
#include "Perceptions.h"

class Person;

class Health_Belief_Model : public Cognitive_Model {
 public:
  Health_Belief_Model(Person *p);
  void reset();
  void update(int day);
  bool will_stay_home_if_sick()  { return stay_home_if_sick; }
  bool will_accept_vaccine() { return accept_vaccine; }
  bool will_avoid_crowds() { return avoid_crowds; }
  bool will_keep_kids_home() { return keep_kids_home; }
  bool will_wear_face_mask() { return wear_face_mask; }
 private:
  void get_parameters();
  void setup_decision_logistic(int action);
  bool get_decision_logistic(int action);

  // Pointer to agent
  Person * self;

  // Memory
  double * memory_susceptibility;
  double * memory_severity;
  double memory_strength;
  int total_deaths;

  // Perceptions
  Perceptions * perceptions;
  int * susceptibility;				/* per strain */
  int * severity;				/* per strain */
  double * benefits_stay_home_if_sick;
  double * benefits_accept_vaccine;
  double * benefits_avoid_crowds;
  double * benefits_keep_kids_home;
  double * benefits_wear_face_mask;
  double * barriers_stay_home_if_sick;
  double * barriers_accept_vaccine;
  double * barriers_avoid_crowds;
  double * barriers_keep_kids_home;
  double * barriers_wear_face_mask;

  // Decisions
  bool stay_home_if_sick;
  bool accept_vaccine;
  bool avoid_crowds;
  bool keep_kids_home;
  bool wear_face_mask;

  bool decide_whether_to_stay_home_if_sick();
  bool decide_whether_to_accept_vaccine();
  bool decide_whether_to_avoid_crowds();
  bool decide_whether_to_keep_kids_home();
  bool decide_whether_to_wear_face_mask();

 protected:
  ~Health_Belief_Model() { }
};

#endif // _FRED_HEALTH_BELIEF_MODEL_H


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

#include "Behavior_Model.h"
#include "Perceptions.h"

class Person;

class Health_Belief_Model : public Behavior_Model {
  public:
    Health_Belief_Model(Person *p);
    void reset();
    void update(int day);
    bool will_accept_vaccine(int disease) {
      return accept_vaccine[disease];
      }
    bool will_accept_another_vaccine_dose(int disease) {
      return true;  // Not implemented
      }
  private:
    void get_parameters();
    bool decide_whether_to_accept_vaccine(int disease);

    // Pointer to agent
    Person * self;

    // Memory
    double * cumm_susceptibility;     // per disease
    double * cumm_severity;     // per disease
    double memory_decay;
    int total_deaths;

    // Perceptions
    Perceptions * perceptions;
    int * perceived_susceptibility;   // per disease
    int * perceived_severity;     // per disease
    double * perceived_benefits_accept_vaccine; // per disease
    double * perceived_barriers_accept_vaccine; // per disease

    // Thresholds for dichotomous variables
    double susceptibility_threshold;
    double severity_threshold;
    double benefits_threshold;
    double barriers_threshold;

    // Decisions
    bool * accept_vaccine;      // per disease

  protected:
    ~Health_Belief_Model() { }
  };

#endif // _FRED_HEALTH_BELIEF_MODEL_H


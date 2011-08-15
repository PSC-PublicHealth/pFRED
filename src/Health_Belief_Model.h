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
  /**
   * Default constructor
   */
  Health_Belief_Model(Person *p);

  /**
   * Perform the daily update for this object
   *
   * @param day the simulation day
   */
  void update(int day);

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept vaccine, <code>false</code> if not
   * @see Behavior_Model::will_accept_vaccine(int disease)
   */
  bool will_accept_vaccine(int disease) { return accept_vaccine[disease]; }

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept another vaccine dose, <code>false</code> if not
   * @see Behavior_Model::will_accept_another_vaccine_dose(int disease)
   */
  bool will_accept_another_vaccine_dose(int disease) { return true; } // Not implemented
 private:
  /**
   * Set the values from the parameter file
   */
  void get_parameters();

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept vaccine, <code>false</code> if not
   */
  bool decide_whether_to_accept_vaccine(int disease);

  // Pointer to agent
  Person * self;

  // Memory
  double * cumm_susceptibility;			// per disease
  double * cumm_severity;			// per disease
  double memory_decay;
  int total_deaths;

  // Perceptions
  Perceptions * perceptions;
  int * perceived_susceptibility;		// per disease
  int * perceived_severity;			// per disease
  double * perceived_benefits_accept_vaccine;	// per disease
  double * perceived_barriers_accept_vaccine;	// per disease
  
  // Thresholds for dichotomous variables
  double susceptibility_threshold;
  double severity_threshold;
  double benefits_threshold;
  double barriers_threshold;

  // Decisions
  bool * accept_vaccine;			// per disease

 protected:
  ~Health_Belief_Model() { }
};

#endif // _FRED_HEALTH_BELIEF_MODEL_H


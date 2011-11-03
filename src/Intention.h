/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Intention.h
//

#ifndef _FRED_INTENTION_H
#define _FRED_INTENTION_H

#include "Random.h"
#include "Behavior.h"

// class Person;
#include "Person.h"

class Place;

class Intention {
 public:
  /**
   * Default constructor
   */
  Intention(Person * self);
  ~Intention() {}

  /**
    * Perform the daily update for this object
    *
    * @param day the simulation day
    */
  void update(int day);

  // access functions
  void set_type(Behavior_type t) { type = t; }
  void set_probability(double p) { probability = p; }
  void set_frequency(int _frequency) { frequency = _frequency; }
  void set_willing(bool decision) { willing = decision; }
  void set_params(Behavior_params * _params) { params = _params; }
  void set_survey(Behavior_survey * _survey) { survey = _survey; }
  int get_type() { return type; }
  double get_probability () { return probability; }
  double get_frequency () { return frequency; }
  bool is_willing() { return willing; }

 private:
  Person * self;
  Behavior_type type;
  Behavior_params * params;
  Behavior_survey * survey;
  double probability;
  int frequency;
  int expiration;
  bool willing;
  
  // private methods
  void reset_survey(int day);
  void update_survey();
  void record_survey_response(Place * place);
  void imitate();
};

inline
void Intention::update(int day) {

  // reset the survey if needed (once per day)
  if (params->imitation_enabled && survey->last_update < day) {
    reset_survey(day);
  }

  if (frequency > 0 && expiration <= day) {
    if (type == IMITATION && day > 0)
      imitate();
    double r = RANDOM();
    willing = (r < probability);
    expiration = day + frequency;
  }

  // respond to survey if any agent is using imitation
  if (params->imitation_enabled) {
    update_survey();
  }
  // printf("INTENT day %d behavior %s person %d type %d willing %d\n", day, params->name, self->get_id(), type, willing?1:0); 
}

#endif // _FRED_INTENTION_H


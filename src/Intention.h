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

class Person;

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
  void update_probabilistically(int day);

  // access functions
  void set_type(Behavior_type t) { type = t; }
  void set_probability(double p) { probability = p; }
  void set_frequency(int interval) { frequency = interval; }
  int get_type() { return type; }
  double get_probability () { return probability; }
  double get_frequency () { return frequency; }
  bool is_willing() { return willing; }
  void set_willing(bool decision) { willing = decision; }

 private:
  Person * self;
  Behavior_type type;
  double probability;
  int frequency;
  int expiration;
  bool willing;
};

inline
void Intention::update(int day) {
  if (frequency > 0) {
    if (type == COIN_TOSS)
      update_probabilistically(day);
  }
}

inline
void Intention::update_probabilistically(int day) {
  if (expiration <= day) {
    willing = (RANDOM() < probability);
    expiration = day + frequency;
  }
}

#endif // _FRED_INTENTION_H


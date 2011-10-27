/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.h
//

#ifndef _FRED_BEHAVIOR_H
#define _FRED_BEHAVIOR_H

#include <vector>

class Person;
class Intention;

enum Behavior_type {REFUSE, ACCEPT, COIN_TOSS, NETWORK, HBM};
#define BEHAVIOR_TYPES 5

typedef struct {
  int enabled;
  double min_prob;
  double max_prob;
  int frequency;
  int cdf_size;
  double cdf[BEHAVIOR_TYPES];
} Behavior_params;


class Behavior {

public:

  /**
   * Constructor
   * @param p a pointer to the agent who will exhibit this behavior
   */
  Behavior(Person *p);
  ~Behavior();
  void initialize_adult_behavior(Person * person);

  /**
    * Perform the daily update for this object
    *
    * @param day the simulation day
    */
  void update(int day);

  bool adult_is_staying_home(int day);
  bool child_is_staying_home(int day);
  bool acceptance_of_vaccine();
  bool acceptance_of_another_vaccine_dose();
  void terminate(){}

private:
  Person * self;
  bool is_child;
  void get_parameters();
  void get_parameters_for_behavior(char * behavior_name, Behavior_params * par);
  Intention * setup(Person * self, Behavior_params params);
  
  Intention * adult_stays_home;
  Intention * child_stays_home;
  Intention * accepts_vaccine;
  Intention * accepts_vaccine_dose;

  static bool parameters_are_set;

  // run-time parameters for probabilistic behaviors
  static Behavior_params adult_stays_home_params;
  static Behavior_params child_stays_home_params;
  static Behavior_params accepts_vaccine_params;
  static Behavior_params accepts_vaccine_dose_params;
  static int number_of_vaccines;
};

#endif // _FRED_BEHAVIOR_H


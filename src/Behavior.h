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

#include <stdio.h>
#include <vector>

class Person;
class Attitude;

enum Behavior_strategy {REFUSE, ACCEPT, FLIP, IMITATE, HBM};
#define BEHAVIOR_STRATEGIES 5
#define NUMBER_WEIGHTS 5

typedef struct {
  int enabled;
  double min_prob;
  double max_prob;
  int frequency;
  int strategy_cdf_size;
  double strategy_cdf[BEHAVIOR_STRATEGIES];
  int strategy_dist[BEHAVIOR_STRATEGIES];
  double weight[NUMBER_WEIGHTS];
  double total_weight;
  double update_rate;
  int imitation_mode;
  double imitation_threshold;
  int imitation_count;
  int imitation_enabled;
  char name[32];
  int first;
} Behavior_params;

typedef struct {
  int * yes_responses;
  int * total_responses;
  int yes;
  int total;
  int * previous_yes_responses;
  int * previous_total_responses;
  int previous_yes;
  int previous_total;
  int last_update;
} Behavior_survey;


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
  void set_parental_decision_maker(Person * person) { parental_decision_maker = person; }
  Person * get_parental_decision_maker() { return parental_decision_maker; }

private:
  Person * self;
  Person * parental_decision_maker;
  void get_parameters();
  void get_parameters_for_behavior(char * behavior_name, Behavior_params * par);
  Attitude * setup(Person * self, Behavior_params * params, Behavior_survey * survey);
  void report_distribution(Behavior_params * params);
  
  Attitude * adult_stays_home;
  Attitude * child_stays_home;
  Attitude * accepts_vaccine;
  Attitude * accepts_vaccine_dose;

  static bool parameters_are_set;
  static int number_of_vaccines;

  // run-time parameters for behaviors
  static Behavior_params adult_stays_home_params;
  static Behavior_params child_stays_home_params;
  static Behavior_params accepts_vaccine_params;
  static Behavior_params accepts_vaccine_dose_params;
  
  static Behavior_survey adult_stays_home_survey;
  static Behavior_survey child_stays_home_survey;
  static Behavior_survey accepts_vaccine_survey;
  static Behavior_survey accepts_vaccine_dose_survey;
  
};

inline
void Behavior::report_distribution(Behavior_params * params) {
  if (params->first) {
    printf("BEHAVIOR %s dist: ", params->name);
    for (int i = 0; i < BEHAVIOR_STRATEGIES; i++) {
      printf("%d ", params->strategy_dist[i]);
    }
    printf("\n"); fflush(stdout);
    params->first = 0;
  }
}

#endif // _FRED_BEHAVIOR_H


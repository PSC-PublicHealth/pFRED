/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.cc
//

#include "Behavior.h"
#include "Person.h"
#include "Global.h"
#include "Attitude.h"
#include "Random.h"
#include "Params.h"
#include "Place_List.h"

//Private static variables that will be set by parameter lookups
int Behavior::number_of_vaccines = 0;
Behavior_params Behavior::adult_stays_home_params = {};
Behavior_params Behavior::child_stays_home_params = {};
Behavior_params Behavior::accepts_vaccine_params = {};
Behavior_params Behavior::accepts_vaccine_dose_params = {};

Behavior_survey Behavior::adult_stays_home_survey = {NULL, NULL, 0,0,NULL, NULL, 0,0,-1};
Behavior_survey Behavior::child_stays_home_survey = {NULL, NULL, 0,0,NULL, NULL, 0,0,-1};
Behavior_survey Behavior::accepts_vaccine_survey = {NULL, NULL, 0,0,NULL, NULL, 0,0,-1};
Behavior_survey Behavior::accepts_vaccine_dose_survey = {NULL, NULL, 0,0,NULL, NULL, 0,0,-1};

//Private static variable to assure we only lookup parameters once
bool Behavior::parameters_are_set = false;

Behavior::Behavior(Person * person) {
  self = person;

  // initialize to null behaviors
  adult_stays_home = NULL;
  child_stays_home = NULL;
  accepts_vaccine = NULL;
  accepts_vaccine_dose = NULL;
  parental_decision_maker = NULL;

  // get parameters (just once)
  if (Behavior::parameters_are_set == false) {
    get_parameters();
  }

  // adults make their own decisions
  if (self->is_adult()) {
    initialize_adult_behavior(person);
  }
}


Behavior::~Behavior() {
  if (adult_stays_home != NULL) delete adult_stays_home;
  if (child_stays_home != NULL) delete child_stays_home;
  if (accepts_vaccine != NULL) delete accepts_vaccine;
  if (accepts_vaccine_dose != NULL) delete accepts_vaccine_dose;
}


void Behavior::initialize_adult_behavior(Person * person) {
  if (adult_stays_home_params.enabled) {
    adult_stays_home = setup(self, &adult_stays_home_params, &adult_stays_home_survey);
  }
  
  if (child_stays_home_params.enabled) {
    child_stays_home = setup(self, &child_stays_home_params, &child_stays_home_survey);
  }
  
  if (Behavior::number_of_vaccines > 0) {
    if (accepts_vaccine_params.enabled) {
      accepts_vaccine = setup(self, &accepts_vaccine_params, &accepts_vaccine_survey);
    }
    if (accepts_vaccine_dose_params.enabled) {
      accepts_vaccine_dose = setup(self, &accepts_vaccine_dose_params, &accepts_vaccine_dose_survey);
    }
  }

  // adult make their own decisions
  parental_decision_maker = self;
}



Attitude * Behavior::setup(Person * self, Behavior_params * params, Behavior_survey * survey) {
  double prob;
  int frequency;
  Attitude * behavior = new Attitude(self);
  int strategy_index = draw_from_distribution(params->strategy_cdf_size, params->strategy_cdf);
  params->strategy_dist[strategy_index]++;
  Behavior_strategy strategy = (Behavior_strategy) strategy_index;
  behavior->set_strategy(strategy);
  // printf("behavior %s setup strategy = %d\n", params->name, strategy);

  switch (strategy) {
  case REFUSE:
    behavior->set_strategy(REFUSE);
    behavior->set_willing(false);
    behavior->set_frequency(0);
    behavior->set_params(params);
    break;
  case ACCEPT:
    behavior->set_strategy(ACCEPT);
    behavior->set_willing(true);
    behavior->set_frequency(0);
    behavior->set_params(params);
    break;
  case FLIP:
    behavior->set_strategy(FLIP);
    prob = URAND(params->min_prob, params->max_prob); 
    behavior->set_probability(prob);
    if (params->frequency > 0) 
      frequency = IRAND(1, params->frequency);
    else
      frequency = IRAND(1, params->frequency);
    behavior->set_willing(RANDOM() < prob);
    behavior->set_frequency(frequency);
    behavior->set_params(params);
    break;
  case IMITATE:
    params->imitation_enabled = 1;
    behavior->set_strategy(IMITATE);
    behavior->set_survey(survey);
    prob = URAND(params->min_prob, params->max_prob); 
    // printf("min_prob %f  max_prob %f  prob %f\n", params->min_prob, params->max_prob, prob);
    behavior->set_probability(prob);
    if (params->frequency > 0) 
      frequency = IRAND(1, params->frequency);
    else
      frequency = IRAND(1, params->frequency);
    behavior->set_willing(RANDOM() < prob);
    behavior->set_frequency(frequency);
    behavior->set_params(params);
    break;
  case HBM:
    behavior->set_strategy(HBM);
    prob = URAND(params->min_prob, params->max_prob); 
    behavior->set_probability(prob);
    if (params->frequency > 0) 
      frequency = IRAND(1, params->frequency);
    else
      frequency = IRAND(1, params->frequency);
    behavior->set_willing(RANDOM() < prob);
    behavior->set_frequency(frequency);
    behavior->set_params(params);
    break;
  }
  return behavior;
}


void Behavior::get_parameters() {
  if (Behavior::parameters_are_set == true) return;
  get_parameters_for_behavior((char *) "adult_stays_home", &adult_stays_home_params);
  get_parameters_for_behavior((char *) "child_stays_home", &child_stays_home_params);
  get_parameters_for_behavior((char *) "accepts_vaccine", &accepts_vaccine_params);
  get_parameters_for_behavior((char *) "accepts_vaccine_dose", &accepts_vaccine_dose_params);
  Behavior::parameters_are_set = true;
}

void Behavior::get_parameters_for_behavior(char * behavior_name, Behavior_params * params) {
  char param_str[256];

  sprintf(param_str, "%s_enabled", behavior_name);
  Params::get_param(param_str, &(params->enabled));

  // probabilistic behavior parameters
  sprintf(param_str, "%s_min_prob", behavior_name);
  Params::get_param(param_str, &(params->min_prob));

  sprintf(param_str, "%s_max_prob", behavior_name);
  Params::get_param(param_str, &(params->max_prob));

  sprintf(param_str, "%s_frequency", behavior_name);
  Params::get_param(param_str, &(params->frequency));

  sprintf(param_str, "%s_strategy_cdf", behavior_name);
  params->strategy_cdf_size = Params::get_param_vector(param_str , params->strategy_cdf);

  sprintf(param_str, "%s_imitation_weights", behavior_name);
  Params::get_param_vector(param_str , params->weight);

  params->total_weight = 0.0;
  for (int i = 0; i < NUMBER_WEIGHTS; i++)
    params->total_weight += params->weight[i];

  sprintf(param_str, "%s_update_rate", behavior_name);
  Params::get_param(param_str, &(params->update_rate));

  sprintf(param_str, "%s_imitation_threshold", behavior_name);
  Params::get_param(param_str, &(params->imitation_threshold));

  sprintf(param_str, "%s_imitation_mode", behavior_name);
  Params::get_param(param_str, &(params->imitation_mode));

  params->imitation_enabled = 0;
  strcpy(params->name, behavior_name);
  for (int i = 0; i < BEHAVIOR_STRATEGIES; i++)
    params->strategy_dist[i] = 0;
  params->first = 1;
}

void Behavior::update(int day) {

  if (self != parental_decision_maker) return;

  if (adult_stays_home_params.enabled) {
    report_distribution(&adult_stays_home_params);
    adult_stays_home->update(day);
  }

  if (child_stays_home_params.enabled) {
    report_distribution(&child_stays_home_params);
    child_stays_home->update(day);
  }

  if (accepts_vaccine_params.enabled) {
    report_distribution(&accepts_vaccine_params);
    accepts_vaccine->update(day);
  }

  if (accepts_vaccine_dose_params.enabled) {
    report_distribution(&accepts_vaccine_dose_params);
    accepts_vaccine_dose->update(day);
  }
}

bool Behavior::adult_is_staying_home(int day) {
  if (adult_stays_home_params.enabled == false)
    return false;

  assert(adult_stays_home != NULL);
  return adult_stays_home->is_willing();
}

bool Behavior::child_is_staying_home(int day) {
  if (child_stays_home_params.enabled == false)
    return false;

  if (self != parental_decision_maker) {
    return parental_decision_maker->get_behavior()->child_is_staying_home(day);
  }

  // adult deciding
  assert(child_stays_home != NULL);
  return child_stays_home->is_willing();
}

bool Behavior::acceptance_of_vaccine() {
  if (accepts_vaccine_params.enabled == false)
    return true;

  if (self != parental_decision_maker) {
    return parental_decision_maker->get_behavior()->acceptance_of_vaccine();
  }

  // adult deciding
  assert(accepts_vaccine != NULL);
  return accepts_vaccine->is_willing();
}

bool Behavior::acceptance_of_another_vaccine_dose() {
  if (accepts_vaccine_dose_params.enabled == false)
    return true;

  if (self != parental_decision_maker) {
    return parental_decision_maker->get_behavior()->acceptance_of_another_vaccine_dose();
  }

  // adult deciding
  assert(accepts_vaccine_dose != NULL);
  return accepts_vaccine_dose->is_willing();
}


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
#include "Intention.h"
#include "Random.h"
#include "Params.h"

//Private static variables that will be set by parameter lookups
int Behavior::number_of_vaccines = 0;
Behavior_params Behavior::adult_stays_home_params = {0,0,0,0,0,{0.0}};
Behavior_params Behavior::child_stays_home_params = {0,0,0,0,0,{0.0}};
Behavior_params Behavior::accepts_vaccine_params = {0,0,0,0,0,{0.0}};
Behavior_params Behavior::accepts_vaccine_dose_params = {0,0,0,0,0,{0.0}};

//Private static variable to assure we only lookup parameters once
bool Behavior::parameters_are_set = false;

Behavior::Behavior(Person * person) {
  self = person;

  // initialize to null behaviors
  adult_stays_home = NULL;
  child_stays_home = NULL;
  accepts_vaccine = NULL;
  accepts_vaccine_dose = NULL;
  is_child = true;


  // get parameters (just once)
  if (Behavior::parameters_are_set == false) {
    get_parameters();
  }

  // current behaviors are all decided by the head of household (HoH)
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
    adult_stays_home = setup(self, adult_stays_home_params);
  }
  
  if (child_stays_home_params.enabled) {
    child_stays_home = setup(self, child_stays_home_params);
  }
  
  if (Behavior::number_of_vaccines > 0) {
    if (accepts_vaccine_params.enabled) {
      accepts_vaccine = setup(self, accepts_vaccine_params);
    }
    if (accepts_vaccine_dose_params.enabled) {
      accepts_vaccine_dose = setup(self, accepts_vaccine_dose_params);
    }
  }

  is_child = false;
}



Intention * Behavior::setup(Person * self, Behavior_params params) {
  double prob;
  Intention * behavior = new Intention(self);
  int itype = draw_from_distribution(params.cdf_size, params.cdf);
  Behavior_type type = (Behavior_type) itype;
  behavior->set_type(type);

  switch (type) {
  case REFUSE:
    behavior->set_willing(false);
    // printf("set willing = false\n");
    behavior->set_frequency(0);
    break;
  case ACCEPT:
    behavior->set_willing(true);
    // printf("set willing = true\n");
    behavior->set_frequency(0);
    break;
  case COIN_TOSS:
    prob = URAND(params.min_prob, params.max_prob); 
    behavior->set_willing(RANDOM() < prob);
    behavior->set_probability(prob);
    behavior->set_frequency(params.frequency);
    break;
  case NETWORK:
    break;
  case HBM:
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

  sprintf(param_str, "%s_segment_cdf", behavior_name);
  params->cdf_size = Params::get_param_vector(param_str , params->cdf);
}

void Behavior::update(int day) {
  if (is_child) return;

  if (adult_stays_home_params.enabled && adult_stays_home->get_frequency() > 0)
    adult_stays_home->update(day);

  if (child_stays_home_params.enabled && child_stays_home->get_frequency() > 0)
    child_stays_home->update(day);

  if (accepts_vaccine_params.enabled && accepts_vaccine->get_frequency() > 0)
    accepts_vaccine->update(day);

  if (accepts_vaccine_dose_params.enabled && accepts_vaccine_dose->get_frequency() > 0)
    accepts_vaccine_dose->update(day);
}

bool Behavior::adult_is_staying_home(int day) {
  assert(adult_stays_home != NULL);
  return adult_stays_home->is_willing();
}

bool Behavior::child_is_staying_home(int day) {
  if (is_child) {
    // ask head of household
    Person * HoH = self->get_HoH();
    return HoH->get_behavior()->child_is_staying_home(day);
  }

  // adult deciding
  assert(child_stays_home != NULL);
  return child_stays_home->is_willing();
}

bool Behavior::acceptance_of_vaccine() {

  if (is_child) {
    // ask head of household
    Person * HoH = self->get_HoH();
    return HoH->get_behavior()->acceptance_of_vaccine();
  }

  // adult deciding
  assert(accepts_vaccine != NULL);
  return accepts_vaccine->is_willing();
}

bool Behavior::acceptance_of_another_vaccine_dose() {

  if (is_child) {
    // ask head of household
    Person * HoH = self->get_HoH();
    return HoH->get_behavior()->acceptance_of_another_vaccine_dose();
  }

  // adult deciding
  assert(accepts_vaccine_dose != NULL);
  return accepts_vaccine_dose->is_willing();
}


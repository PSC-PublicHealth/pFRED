/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Behavior.cc
//

#include "Global.h"
#include "Behavior.h"
#include "Person.h"
#include "Attitude.h"
#include "Random.h"
#include "Params.h"
#include "Place_List.h"
#include "Utils.h"
#include "Household.h"

//Private static variable to assure we only lookup parameters once
bool Behavior::parameters_are_set = false;
Behavior_params ** Behavior::behavior_params = new Behavior_params * [NUM_BEHAVIORS];


Behavior::Behavior( Person * self ) {

  // get parameters (just once)
  if (Behavior::parameters_are_set == false) {
    for (int i = 0; i < NUM_BEHAVIORS; i++) {
      behavior_params[i] = new Behavior_params;
    }
    get_parameters();
    for (int i = 0; i < NUM_BEHAVIORS; i++) {
      print_params(i);
    }
  }

  // will be initialized in setup_attitudes() if needed
  attitude = NULL;

  // will be properly initialized in setup() after all agents are created
  health_decision_maker = NULL;
}


Behavior::~Behavior() {
  delete_attitudes();
}


void Behavior::delete_attitudes() {
  if ( attitude != NULL ) {
    for (int i = 0; i < NUM_BEHAVIORS; i++) {
      if ( attitude[i] != NULL ) {
        // printf( "%p\n",attitude[i]);
        delete attitude[i];
      }
    }
    delete [] attitude;
    attitude = NULL;
  }
}


void Behavior::setup( Person * self ) {
  if (Global::Enable_Behaviors == 0) return;

  // setup an adult
  if (self->is_adult()) {
    // adults do not have a separate health decision maker
    FRED_VERBOSE(1,"behavior_setup for adult %d age %d -- will make own health decisions\n",
                 self->get_id(), self->get_age());
    health_decision_maker = NULL;
    setup_attitudes();
    return;
  }

  // setup a child
  int relationship = self->get_relationship();
  Household * h = (Household *) self->get_household();
  Person * person = select_adult( h, relationship, self );

  // child is on its own
  if ( person == NULL ) {
    FRED_VERBOSE(1,"behavior_setup for child %d age %d -- will make own health decisions\n",
                 self->get_id(), self->get_age());
    // no separate health decision maker
    health_decision_maker = NULL;
    setup_attitudes();
    return;
  }

  // an older child is available
  if ( person->is_adult() == false ) {
    FRED_VERBOSE(0,"behavior_setup for child %d age %d -- minor person %d age %d will make health decisions\n",
                 self->get_id(), self->get_age(), person->get_id(), person->get_age());
    health_decision_maker = person;
    person->become_health_decision_maker();
    return;
  }

  // an adult is available
  FRED_VERBOSE(0,"behavior_setup for child %d age %d -- adult person %d age %d will make health decisions\n",
               self->get_id(), self->get_age(), person->get_id(), person->get_age());
  health_decision_maker = person; // no need to setup atitudes for adults
  return;
}

void Behavior::setup_attitudes() {
  if (Global::Enable_Behaviors == 0) return;

  assert(attitude == NULL);

  // create array of pointers to attitudes
  attitude = new Attitude * [NUM_BEHAVIORS];

  // initialize to null attitudes
  for (int i = 0; i < NUM_BEHAVIORS; i++) {
    attitude[i] = NULL;
  }

  if ( Behavior::behavior_params[STAY_HOME_WHEN_SICK]->enabled ) {
    attitude[STAY_HOME_WHEN_SICK] = new Attitude(STAY_HOME_WHEN_SICK);
  }
  if ( Behavior::behavior_params[TAKE_SICK_LEAVE]->enabled ) {
    attitude[TAKE_SICK_LEAVE] = new Attitude(TAKE_SICK_LEAVE);
  }
  if ( Behavior::behavior_params[KEEP_CHILD_HOME_WHEN_SICK]->enabled ) {
    attitude[KEEP_CHILD_HOME_WHEN_SICK] = new Attitude(KEEP_CHILD_HOME_WHEN_SICK);
  }
  if ( Behavior::behavior_params[ACCEPT_VACCINE]->enabled ) {
    attitude[ACCEPT_VACCINE] = new Attitude(ACCEPT_VACCINE);
  }
  if ( Behavior::behavior_params[ACCEPT_VACCINE_DOSE]->enabled ) {
    attitude[ACCEPT_VACCINE_DOSE] = new Attitude(ACCEPT_VACCINE_DOSE);
  }
  if ( Behavior::behavior_params[ACCEPT_VACCINE_FOR_CHILD]->enabled ) {
    attitude[ACCEPT_VACCINE_FOR_CHILD] = new Attitude(ACCEPT_VACCINE_FOR_CHILD);
  }
  if ( Behavior::behavior_params[ACCEPT_VACCINE_DOSE_FOR_CHILD]->enabled ) {
    attitude[ACCEPT_VACCINE_DOSE_FOR_CHILD] = new Attitude(ACCEPT_VACCINE_DOSE_FOR_CHILD);
  }
}


void Behavior::get_parameters() {
  if (Behavior::parameters_are_set == true) return;
  get_parameters_for_behavior((char *) "stay_home_when_sick", STAY_HOME_WHEN_SICK);
  get_parameters_for_behavior((char *) "take_sick_leave", TAKE_SICK_LEAVE);
  get_parameters_for_behavior((char *) "keep_child_home_when_sick", KEEP_CHILD_HOME_WHEN_SICK);
  get_parameters_for_behavior((char *) "accept_vaccine", ACCEPT_VACCINE);
  get_parameters_for_behavior((char *) "accept_vaccine_dose", ACCEPT_VACCINE_DOSE);
  get_parameters_for_behavior((char *) "accept_vaccine_for_child", ACCEPT_VACCINE_FOR_CHILD);
  get_parameters_for_behavior((char *) "accept_vaccine_dose_for_child", ACCEPT_VACCINE_DOSE_FOR_CHILD);
  Behavior::parameters_are_set = true;
}

void Behavior::get_parameters_for_behavior(char * behavior_name, int j) {
  Behavior_params * params = Behavior::behavior_params[j];
  strcpy(params->name, behavior_name);

  char param_str[FRED_STRING_SIZE];
  sprintf(param_str, "%s_enabled", behavior_name);
  Params::get_param(param_str, &(params->enabled));

  for (int i = 0; i < NUM_BEHAVIOR_STRATEGIES; i++)
    params->strategy_population[i] = 0;

  sprintf(param_str, "%s_strategy_distribution", behavior_name);
  params->strategy_cdf_size = Params::get_param_vector(param_str , params->strategy_cdf);
  
  // convert to cdf
  double stotal = 0;
  for (int i = 0; i < params->strategy_cdf_size; i++) stotal += params->strategy_cdf[i];
  if (stotal != 100.0 && stotal != 1.0) {
    Utils::fred_abort("Bad distribution %s params_str\nMust sum to 1.0 or 100.0\n", param_str);
  }
  double cumm = 0.0;
  for (int i = 0; i < params->strategy_cdf_size; i++){
    params->strategy_cdf[i] /= stotal;
    params->strategy_cdf[i] += cumm;
    cumm = params->strategy_cdf[i];
  }

  printf("BEHAVIOR %s strategy_cdf: ", params->name);
  for (int i = 0; i < NUM_BEHAVIOR_STRATEGIES; i++) {
    printf("%f ", params->strategy_cdf[i]);
  }
  printf("\n"); fflush(stdout);

  sprintf(param_str, "%s_frequency", behavior_name);
  Params::get_param(param_str, &(params->frequency));

  // FLIP behavior parameters

  sprintf(param_str, "%s_min_prob", behavior_name);
  Params::get_param(param_str, &(params->min_prob));

  sprintf(param_str, "%s_max_prob", behavior_name);
  Params::get_param(param_str, &(params->max_prob));

  // IMITATE PREVALENCE behavior parameters

  sprintf(param_str, "%s_imitate_prevalence_weights", behavior_name);
  Params::get_param_vector(param_str , params->imitate_prevalence_weight);

  params->imitate_prevalence_total_weight = 0.0;
  for (int i = 0; i < NUM_WEIGHTS; i++)
    params->imitate_prevalence_total_weight += params->imitate_prevalence_weight[i];

  sprintf(param_str, "%s_imitate_prevalence_update_rate", behavior_name);
  Params::get_param(param_str, &(params->imitate_prevalence_update_rate));

  // IMITATE CONSENSUS behavior parameters

  sprintf(param_str, "%s_imitate_consensus_weights", behavior_name);
  Params::get_param_vector(param_str , params->imitate_consensus_weight);

  params->imitate_consensus_total_weight = 0.0;
  for (int i = 0; i < NUM_WEIGHTS; i++)
    params->imitate_consensus_total_weight += params->imitate_consensus_weight[i];

  sprintf(param_str, "%s_imitate_consensus_update_rate", behavior_name);
  Params::get_param(param_str, &(params->imitate_consensus_update_rate));

  sprintf(param_str, "%s_imitate_consensus_threshold", behavior_name);
  Params::get_param(param_str, &(params->imitate_consensus_threshold));

  // IMITATE COUNT behavior parameters

  sprintf(param_str, "%s_imitate_count_weights", behavior_name);
  Params::get_param_vector(param_str , params->imitate_count_weight);

  params->imitate_count_total_weight = 0.0;
  for (int i = 0; i < NUM_WEIGHTS; i++)
    params->imitate_count_total_weight += params->imitate_count_weight[i];

  sprintf(param_str, "%s_imitate_count_update_rate", behavior_name);
  Params::get_param(param_str, &(params->imitate_count_update_rate));

  sprintf(param_str, "%s_imitate_count_threshold", behavior_name);
  Params::get_param(param_str, &(params->imitate_count_threshold));

  params->imitation_enabled = 0;

  // HBM parameters

  sprintf(param_str, "%s_susceptibility_threshold", behavior_name);
  int n = Params::get_param_vector(param_str, params->susceptibility_threshold_distr);
  if (n != 2) { 
    Utils::fred_abort("bad %s\n", param_str); 
  }
  
  sprintf(param_str, "%s_severity_threshold", behavior_name);
  n = Params::get_param_vector(param_str, params->severity_threshold_distr);
  if (n != 2) {  
    Utils::fred_abort("bad %s\n", param_str); 
  }
  
  sprintf(param_str, "%s_benefits_threshold", behavior_name);
  n = Params::get_param_vector(param_str, params->benefits_threshold_distr);
  if (n != 2) { 
    Utils::fred_abort("bad %s\n", param_str); 
  }
  
  sprintf(param_str, "%s_barriers_threshold", behavior_name);
  n = Params::get_param_vector(param_str, params->barriers_threshold_distr);
  if (n != 2) {  
    Utils::fred_abort("bad %s\n", param_str); 
  }
  
  sprintf(param_str, "%s_memory_decay", behavior_name);
  n = Params::get_param_vector(param_str, params->memory_decay_distr);
  if (n != 2) {  
    Utils::fred_abort("bad %s\n", param_str);  
  }
  
  sprintf(param_str, "%s_base_odds_ratio", behavior_name);
  Params::get_param(param_str, &(params->base_odds_ratio));

  sprintf(param_str, "%s_susceptibility_odds_ratio", behavior_name);
  Params::get_param(param_str, &(params->susceptibility_odds_ratio));

  sprintf(param_str, "%s_severity_odds_ratio", behavior_name);
  Params::get_param(param_str, &(params->severity_odds_ratio));

  sprintf(param_str, "%s_benefits_odds_ratio", behavior_name);
  Params::get_param(param_str, &(params->benefits_odds_ratio));

  sprintf(param_str, "%s_barriers_odds_ratio", behavior_name);
  Params::get_param(param_str, &(params->barriers_odds_ratio));
}

void Behavior::print_params(int n) {
  Behavior_params * params = Behavior::behavior_params[n];
  printf("PRINT BEHAVIOR PARAMS:\n");
  printf("name = %s\n", params->name);
  printf("enabled = %d\n", params->enabled);
  printf("frequency = %d\n", params->frequency);
  printf("strategy_population = ");
  for (int i = 0; i < NUM_BEHAVIOR_STRATEGIES; i++) {
    printf("%d ", params->strategy_population[i]);
  }
  printf("\n");
  fflush(stdout);
}

void Behavior::update( Person * self, int day ) {

  FRED_VERBOSE(1,"behavior::update person %d day %d\n", self->get_id(), day);

  if (day == -1 && self->get_id() == 0) {
    for (int i = 0; i < NUM_BEHAVIORS; i++)
      print_params(i);
  }

  if (Global::Enable_Behaviors == 0) return;
  if (health_decision_maker != NULL) return;

  for (int i = 0; i < NUM_BEHAVIORS; i++) {
    Behavior_params * params = Behavior::behavior_params[i];
    if (params->enabled) {
      FRED_VERBOSE(1,"behavior::update update attitude[%d]\n", i);
      assert(attitude[i] != NULL);
      attitude[i]->update(day);
    }
  }
  FRED_VERBOSE(1,"behavior::update complete person %d day %d\n", self->get_id(), day);
}

bool Behavior::adult_is_staying_home() {
  assert(Global::Enable_Behaviors > 0);
  int n = STAY_HOME_WHEN_SICK;
  Attitude *att = attitude[n];
  if (att == NULL) return false;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  return att->is_willing();
}

bool Behavior::adult_is_taking_sick_leave() {
  assert(Global::Enable_Behaviors > 0);
  int n = TAKE_SICK_LEAVE;
  Attitude *att = attitude[n];
  if (att == NULL) return false;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  return att->is_willing();
}

bool Behavior::child_is_staying_home() {
  assert(Global::Enable_Behaviors > 0);
  int n = KEEP_CHILD_HOME_WHEN_SICK;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  if (health_decision_maker != NULL) {
    return health_decision_maker->child_is_staying_home(  );
  }
  // I am the health decision maker
  Attitude *att = attitude[n];
  assert (att != NULL);
  return att->is_willing();
}

bool Behavior::acceptance_of_vaccine() {
  assert(Global::Enable_Behaviors > 0);
  int n = ACCEPT_VACCINE;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  if (health_decision_maker != NULL) {
    return health_decision_maker->acceptance_of_vaccine();
  }
  // I am the health decision maker
  Attitude *att = attitude[n];
  assert (att != NULL);
  return att->is_willing();
}

bool Behavior::acceptance_of_another_vaccine_dose() {
  assert(Global::Enable_Behaviors > 0);
  int n = ACCEPT_VACCINE_DOSE;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  if (health_decision_maker != NULL) {
    return health_decision_maker->acceptance_of_another_vaccine_dose();
  }
  // I am the health decision maker
  Attitude *att = attitude[n];
  assert (att != NULL);
  return att->is_willing();
}

bool Behavior::child_acceptance_of_vaccine() {
  assert(Global::Enable_Behaviors > 0);
  int n = ACCEPT_VACCINE_FOR_CHILD;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  if (health_decision_maker != NULL) {
    return health_decision_maker->child_acceptance_of_vaccine();
  }
  // I am the health decision maker
  Attitude *att = attitude[n];
  assert (att != NULL);
  return att->is_willing();
}

bool Behavior::child_acceptance_of_another_vaccine_dose() {
  assert(Global::Enable_Behaviors > 0);
  int n = ACCEPT_VACCINE_DOSE_FOR_CHILD;
  Behavior_params * params = Behavior::behavior_params[n];
  if (params->enabled == false) return false;
  if (health_decision_maker != NULL) {
    return health_decision_maker->child_acceptance_of_another_vaccine_dose();
  }
  // I am the health decision maker
  Attitude *att = attitude[n];
  assert (att != NULL);
  return att->is_willing();
}

Person * Behavior::select_adult(Household *h, int relationship, Person * self) {

  int N = h->get_size();

  if (relationship == Global::GRANDCHILD) {

    // select first adult natural child or spouse thereof of householder parent, if any
    for (int i = 0; i < N; i++) {
      Person * person = h->get_housemate(i);
      if (person->is_adult() == false || person == self)
        continue;
      int r = person->get_relationship();
      if (r == Global::SPOUSE || r == Global::CHILD || r == Global::SIBLING || r == Global::IN_LAW) {
        return person;
      }
    }

    // consider adult relative of householder, if any
    for (int i = 0; i < N; i++) {
      Person * person = h->get_housemate(i);
      if (person->is_adult() == false || person == self)
        continue;
      int r = person->get_relationship();
      if (r == Global::PARENT || r == Global::OTHER_RELATIVE) {
        return person;
      }
    }
  }

  // select householder if an adult
  for (int i = 0; i < N; i++) {
    Person * person = h->get_housemate(i);
    if (person->is_adult() == false || person == self)
      continue;
    if (person->get_relationship() == Global::HOUSEHOLDER) {
      return person;
    }
  }
  
  // select householder's spouse if an adult
  for (int i = 0; i < N; i++) {
    Person * person = h->get_housemate(i);
    if (person->is_adult() == false || person == self)
      continue;
    if (person->get_relationship() == Global::SPOUSE) {
      return person;
    }
  }

  // select oldest available person
  int max_age = self->get_age();;
  Person * oldest = NULL;
  for (int i = 0; i < N; i++) {
    Person * person = h->get_housemate(i);
    if (person->get_age() <= max_age || person == self)
      continue;
    oldest = person;
    max_age = oldest->get_age();
  }
  return oldest;
}

void Behavior::terminate( Person * self ) {
  if (Global::Enable_Behaviors == 0) return;
  if (health_decision_maker != NULL)
    return;
  if (Global::Verbose > 1) {
    printf("terminating behavior for agent %d age %d\n",
     self->get_id(), self->get_age());
  }

  // see if this person is the adult decision maker for any child in the household
  Household * household = (Household *) self->get_household();
  int size = household->get_size();
  for (int i = 0; i < size; i++) {
    Person * child = household->get_housemate(i);
    if (child != self && child->get_health_decision_maker() == self) {
      if (Global::Verbose > 1) {
        printf("need new decision maker for agent %d age %d\n",
            child->get_id(), child->get_age());
      }
      child->setup_behavior();
      if (Global::Verbose > 1) {
        printf("new decision maker is %d age %d\n",
            child->get_health_decision_maker()->get_id(),
            child->get_health_decision_maker()->get_age());
        fflush(stdout);
      }
    }
  }
}
  

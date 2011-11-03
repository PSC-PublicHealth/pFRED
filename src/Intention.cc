/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Intention.cc
//

#include "Intention.h"
#include "Global.h"
#include "Place_List.h"
#include "Place.h"
#include "Person.h"

Intention::Intention(Person * _person) {
  self = _person;
  type = REFUSE;
  probability = 0.0;
  frequency = 0;
  expiration = 0;
  willing = false;
}

////// IMITATION BEHAVIOR

void Intention::reset_survey(int day) {
  int places = Global::Places.get_number_of_places();

  // allocate space for survey if needed
  if (survey->yes_responses == NULL) {
    printf("SURVEY for %s allocated on day %d for %d places\n",
	   params->name,day,places); fflush(stdout);
    survey->yes_responses = new int [places];
    assert (survey->yes_responses != NULL);
    survey->total_responses = new int [places];
    assert (survey->total_responses != NULL);
    survey->previous_yes_responses = new int [places];
    assert (survey->previous_yes_responses != NULL);
    survey->previous_total_responses = new int [places];
    assert (survey->previous_total_responses != NULL);
    printf("SURVEY allocated on day %d for %d places\n", day,places); fflush(stdout);
  }

  // swap previous and current day's surveys
  int * tmp = survey->yes_responses;
  survey->yes_responses = survey->previous_yes_responses;
  survey->previous_yes_responses = tmp;
  survey->previous_yes = survey->yes;
  survey->previous_total = survey->total;
  
  if (day > 0) {
    printf("SURVEY of %s for day %d YES %d TOT %d PREVALENCE %f\n",
	   params->name,day, survey->yes, survey->total,
	   survey->total?(survey->yes/(double)survey->total):0.0);
  }

  // clear out today's survey
  for (int i = 0; i < places; i++) {
    // printf("SURVEY resetting for day %d index %d\n", day,i); fflush(stdout);
    survey->yes_responses[i] = 0;
    survey->total_responses[i] = 0;
  }
  survey->yes = 0;
  survey->total = 0;

  // mark this day's survey as reset
  survey->last_update = day;
  // printf("SURVEY RESET for day %d\n", day); fflush(stdout);
}

void Intention::update_survey() {
  if (params->w[0] != 0.0) {
    record_survey_response(self->get_household());
  }
  if (params->w[1] != 0.0) {
    record_survey_response(self->get_neighborhood());
  }
  if (params->w[2] != 0.0) {
    record_survey_response(self->get_workplace());
  }
  if (params->w[3] != 0.0) {
    record_survey_response(self->get_school());
  }
  if (params->w[4] != 0.0) {
    record_survey_response(NULL);
  }
}

void Intention::record_survey_response(Place * place) {
  if (place != NULL) {
    int place_id = place->get_id();
    if (willing) {
      survey->yes_responses[place_id]++;
    }
    survey->total_responses[place_id]++;
  }
  else {
    if (willing) {
      survey->yes++;
    }
    survey->total++;
  }
}

void Intention::imitate() {
  Place * place;
  int place_id;
  double * weight = params->w;
  int * yes_responses = survey->previous_yes_responses;
  int * total_responses = survey->previous_total_responses;
  double weighted_sum = 0.0;
  if (params->total_weight == 0.0) return;
  // printf("total weight = %f\n", params->total_weight);
  
  // process household preferences
  if (weight[0] != 0.0) {
    place = self->get_household();
    if (place != NULL) {
      int place_id = place->get_id();
      if (total_responses[place_id] > 0) {
	double contribution = weight[0] * yes_responses[place_id];
	int mode = params->imitation_mode;
	if (mode == 0 || mode == 1 || mode == 3)
	  contribution /= total_responses[place_id];
	weighted_sum += contribution;
      }
    }
  }

  // process neighborhood preferences
  if (weight[1] != 0.0) {
    place = self->get_neighborhood();
    if (place != NULL) {
      place_id = place->get_id();
      if (total_responses[place_id] > 0) {
	double contribution = weight[1] * yes_responses[place_id];
	int mode = params->imitation_mode;
	if (mode == 0 || mode == 1 || mode == 3)
	  contribution /= total_responses[place_id];
	weighted_sum += contribution;
      }
    }
  }

  // process workplace preferences
  if (weight[2] != 0.0) {
    place = self->get_workplace();
    if (place != NULL) {
      place_id = place->get_id();
      if (total_responses[place_id] > 0) {
	double contribution = weight[2] * yes_responses[place_id];
	int mode = params->imitation_mode;
	if (mode == 0 || mode == 1 || mode == 3)
	  contribution /= total_responses[place_id];
	weighted_sum += contribution;
      }
    }
  }

  // process school preferences
  if (weight[3] != 0.0) {
    place = self->get_school();
    if (place != NULL) {
      place_id = place->get_id();
      if (total_responses[place_id] > 0) {
	double contribution = weight[3] * yes_responses[place_id];
	int mode = params->imitation_mode;
	if (mode == 0 || mode == 1 || mode == 3)
	  contribution /= total_responses[place_id];
	weighted_sum += contribution;
      }
    }
  }

  // process community preferences
  if (weight[4] != 0.0) {
    if (params->imitation_mode == 2 || params->imitation_mode == 4)
      weighted_sum += weight[4] * survey->previous_yes;
    else
      weighted_sum += weight[4] *
	(double) survey->previous_yes / (double) survey->previous_total;
  }

  double prevalence = weighted_sum / params->total_weight;

  /*
  printf("imitate person %d yes %d  tot %d ws = %f  w: %f %f %f %f %f ",
	 self->get_id(), survey->yes, survey->total, weighted_sum,
	 weight[0],weight[1],weight[2],weight[3],weight[4]);

  printf("mode %d update_rate %f prevalance %f prob before %f ",
	 params->imitation_mode, params->update_rate, prevalence, probability);

  */

  switch (params->imitation_mode) {
  case 0:
    probability *= (1.0 - params->update_rate);
    probability += params->update_rate * prevalence;
    break;
  case 1:
  case 2:
    if (prevalence > params->imitation_threshold) {
      probability *= (1.0 - params->update_rate);
      probability += params->update_rate;
    }
    break;
  case 3:
  case 4:
    probability *= (1.0 - params->update_rate);
    if (prevalence > params->imitation_threshold) {
      probability += params->update_rate;
    }
    break;
  }
  // printf(" prob after %f\n", probability);
}



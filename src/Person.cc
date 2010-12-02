/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Person.cc
//

#include "Person.h"

#include "Behavior.h"
#include "Demographics.h"
#include "Global.h"
#include "Health.h"
#include "Cognition.h"
#include "Place.h"
#include "Disease.h"
#include "Population.h"
#include "AV_Health.h"
#include "Age_Map.h"

#include <cstdio>

Person::Person() { 
  idx = -1;
  pop = NULL;
  demographics = NULL;
  health = NULL;
  behavior = NULL;
  cognition = NULL;
}

Person::~Person() {
  delete demographics;
  delete health; 
  delete behavior;
  delete cognition;
}

void Person::setup(int index, int age, char sex, int marital,
                   int profession, Place **favorite_places, int profile, Population* Pop) 
{
  idx = index;
  pop = Pop;
  demographics = new Demographics(this, age,sex,marital,profession);
  health = new Health(this);
  behavior = new Behavior(this,favorite_places,profile);
  cognition = new Cognition(this);
}

void Person::print(int disease) const {
  if (Tracefp == NULL) return;
  fprintf(Tracefp, "%i %c id %7d  a %3d  s %c %d ",
          disease, health->get_disease_status(disease), idx,
          demographics->get_age(),
          demographics->get_sex(),
          demographics->get_profession());
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
          health->get_exposure_date(disease), health->get_infectious_date(disease), health->get_recovered_date(disease));
  fprintf(Tracefp, "places %d ", FAVORITE_PLACES);
  fprintf(Tracefp, "infected_at %c %6d ",
          health->get_infected_place_type(disease), health->get_infected_place(disease));
  fprintf(Tracefp, "infector %d ", health->get_infector(disease));
  fprintf(Tracefp, "infectees %d ", health->get_infectees(disease));
  fprintf(Tracefp, "antivirals: %2d ",health->get_number_av_taken());
  for(int i=0;i<health->get_number_av_taken();i++)
    fprintf(Tracefp," %2d",health->get_av_health(i)->get_av_start_day());
  
  
  // fprintf(Tracefp, "vaccines: %2d", health->get_number_vaccines_taken());
  //   for(int i=0;i<health->get_number_vaccines_taken();i++)
  //     fprintf(Tracefp," %2d %2d %2d",health->get_vaccine_stat(i)->get_vaccination_day(),
  // 	    health->get_vaccine_stat(i)->is_effective(),health->get_vaccine_stat(i)->get_current_dose());
  fprintf(Tracefp,"\n");
  fflush(Tracefp);
}

void Person::print_out(int disease) const {
  fprintf(stdout, "%c id %7d  a %3d  s %c %d ",
          health->get_disease_status(disease), idx,
          demographics->get_age(),
          demographics->get_sex(),
          demographics->get_profession());
  fprintf(stdout, "exp: %2d  inf: %2d  rem: %2d ",
          health->get_exposure_date(disease), health->get_infectious_date(disease), health->get_recovered_date(disease));
  fprintf(stdout, "places %d ", FAVORITE_PLACES);
  fprintf(stdout, "infected_at %c %6d ",
          health->get_infected_place_type(disease), health->get_infected_place(disease));
  fprintf(stdout, "infector %d ", health->get_infector(disease));
  fprintf(stdout, "infectees %d\n", health->get_infectees(disease));
  fflush(stdout);
}

void Person::print_schedule() const {
  behavior->print_schedule();
}

void Person::reset() {
  if (Verbose > 2) { fprintf(Statusfp, "reset person %d\n", idx); fflush(Statusfp); }
  demographics->reset();
  health->reset();
  cognition->reset();
  behavior->reset();
	
  for (int disease = 0; disease < Pop.get_diseases(); disease++) {
    Disease* s = Pop.get_disease(disease);
    if (!s->get_residual_immunity()->is_empty()) {
      double residual_immunity_prob = s->get_residual_immunity()->find_value(get_age());
      if (RANDOM() < residual_immunity_prob)
	become_immune(s);
    }
  }
}

void Person::update(int day) {
  demographics->update(day);
  health->update(day);
  cognition->update(day);
  behavior->update(day);
}

void Person::become_susceptible(int disease) {
  health->become_susceptible(disease);
  behavior->become_susceptible(disease);
}

void Person::become_exposed(Infection * infection) {
  health->become_exposed(infection);
  behavior->become_exposed(infection->get_disease()->get_id());
}

void Person::become_infectious(Disease * disease) {
  int disease_id = disease->get_id();
  health->become_infectious(disease);
  behavior->become_infectious(disease_id);
}

void Person::become_symptomatic(Disease *disease) {
	health->become_symptomatic(disease);
}

void Person::become_immune(Disease* disease) {
  int disease_id = disease->get_id();
  char status = health->get_disease_status(disease_id);
  if(status == 'S'){
    health->become_immune(disease);
    behavior->become_immune(disease_id);
  }
}

void Person::recover(Disease * disease) {
  int disease_id = disease->get_id();
  health->recover(disease);
  behavior->recover(disease_id);
  
  if (Verbose > 2) {
    fprintf(Statusfp, "RECOVERED person %d for disease %d\n", idx, disease_id);
    print_out(disease_id);
    fflush(Statusfp);
  }
}

void Person::update_schedule(int day) {
  return behavior->update_schedule(day);
}

void Person::behave(int day) {}

int Person::is_symptomatic() const {
  return health->is_symptomatic();
}

Place * Person::get_household() const {
  return behavior->get_household();
}

int Person::get_age() const { return demographics->get_age(); }

char Person::get_sex() const { return demographics->get_sex(); }

char Person::get_marital_status() const {
  return demographics->get_marital_status();
}

int Person::get_profession() const {
  return demographics->get_profession();
}

int Person::get_exposure_date(int disease) const {
  return health->get_exposure_date(disease);
}

int Person::get_infectious_date(int disease) const {
  return health->get_infectious_date(disease);
}

int Person::get_recovered_date(int disease) const {
  return health->get_recovered_date(disease);
}

int Person::get_infector(int disease) const {
  return health->get_infector(disease);
}

int Person::get_infected_place(int disease) const {
  return health->get_infected_place(disease);
}

char Person::get_infected_place_type(int disease) const {
  return health->get_infected_place_type(disease);
}

int Person::get_infectees(int disease) const {
  return health->get_infectees(disease);
}

int Person::add_infectee(int disease) {
  return health->add_infectee(disease);
}

int Person::is_new_case(int day, int disease) const {
  return (health->get_exposure_date(disease) == day);
}

void Person::set_changed(){
  this->pop->set_changed(this);
}

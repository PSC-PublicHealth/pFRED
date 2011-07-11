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

#include "Activities.h"
#include "Demographics.h"
#include "Global.h"
#include "Health.h"
#include "Behavior.h"
#include "Place.h"
#include "Disease.h"
#include "Population.h"
#include "AV_Health.h"
#include "Age_Map.h"
#include "Date.h"
#include "Place_List.h"
#include "Person_Event_Interface.h"
#include "Transmission.h"

#include <cstdio>
#include <vector>

Person::Person() { 
  idx = -1;
  pop = NULL;
  demographics = NULL;
  health = NULL;
  activities = NULL;
  behavior = NULL;
  registered_event_handlers = NULL;
}

Person::~Person() {
  if (demographics != NULL) delete demographics;
  if (health != NULL) delete health;
  if (activities != NULL) delete activities;
  if (behavior != NULL) delete behavior;
  if (registered_event_handlers != NULL) delete registered_event_handlers;
}

void Person::setup(int index, int age, char sex, int marital, int profession,
		   Place **favorite_places, Population* Pop,
		   Date *sim_start_date, bool has_random_birthday)
{
  idx = index;
  pop = Pop;
  demographics =
    new Demographics(this, age, sex, marital,
		     profession, sim_start_date, has_random_birthday);
  health = new Health(this);
  activities = new Activities(this, favorite_places);
  behavior = new Behavior(this);
}

void Person::setup(int index, int age, char sex, int marital, int occ,
		   char *house, char *school, char *work, Population *Pop,
		   Date *sim_start_date, bool has_random_birthday)
{
  idx = index;
  pop = Pop;
  demographics =
    new Demographics(this, age, sex, marital,
		     occ, sim_start_date, has_random_birthday);
  health = new Health(this);
  activities = new Activities(this, house, school, work);
  behavior = new Behavior(this);
}

void Person::print(FILE *fp, int disease) const {
  if (fp == NULL) return;
  fprintf(fp, "%i %c id %7d  a %3d  s %c %d ",
          disease, health->get_disease_status(disease), idx,
          demographics->get_age(),
          demographics->get_sex(),
          demographics->get_profession());
  fprintf(fp, "exp: %2d  inf: %2d  rem: %2d ",
          health->get_exposure_date(disease),
	  health->get_infectious_date(disease),
	  health->get_recovered_date(disease));
  fprintf(fp, "sympt: %d ", health->get_symptomatic_date(disease));

  fprintf(fp, "places %d ", FAVORITE_PLACES);
  fprintf(fp, "infected_at %c %6d ",
          health->get_infected_place_type(disease),
	  health->get_infected_place(disease));
  fprintf(fp, "infector %d ", health->get_infector(disease));
  fprintf(fp, "infectees %d ", health->get_infectees(disease));
  fprintf(fp, "antivirals: %2d ",health->get_number_av_taken());
  for(int i=0;i<health->get_number_av_taken();i++)
    fprintf(fp," %2d",health->get_av_health(i)->get_av_start_day());
  fprintf(fp,"\n");
  fflush(fp);
}

void Person::reset(Date * sim_start_date) {
  if (Global::Verbose > 2) {
    fprintf(Global::Statusfp, "reset person %d\n", idx);
    fflush(Global::Statusfp);
  }
  demographics->reset(sim_start_date);
  health->reset();
  behavior->reset();
  activities->reset();
	
  for (int disease = 0; disease < Global::Pop.get_diseases(); disease++) {
    Disease* s = Global::Pop.get_disease(disease);
    if (!s->get_residual_immunity()->is_empty()) {
      double residual_immunity_prob =
	s->get_residual_immunity()->find_value(get_age());
      if (RANDOM() < residual_immunity_prob)
	become_immune(s);
    }
  }
}

int Person::get_diseases() {
  return pop->get_diseases();
}

void Person::become_unsusceptible(int disease) {
  health->become_unsusceptible(disease);
}

void Person::become_immune(Disease* disease) {
  int disease_id = disease->get_id();
  char status = health->get_disease_status(disease_id);
  if(status == 'S'){
    health->become_immune(disease);
  }
}

Place * Person::get_household() const {
  return activities->get_household();
}

Place * Person::get_neighborhood() const {
  return activities->get_neighborhood();
}

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

char * Person::get_infected_place_label(int disease) const {
  return health->get_infected_place_label(disease);
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

void Person::infect(Person *infectee, int disease, Transmission *transmission) {
  this->health->infect(infectee, disease, transmission);
  this->add_infectee(disease);
}

void Person::getInfected(Disease *disease, Transmission *transmission) {
  health->getInfected(disease, transmission);
  behavior->getInfected(disease, transmission);
}

Person * Person::give_birth(int day) {
  int id = pop->get_next_id(), age = 0, married = -1, prof = 2;
  char sex = (URAND(0.0, 1.0) < 0.5 ? 'M' : 'F');
  Place *favorite_place[] = {
    get_household(), get_neighborhood(),
    NULL, NULL, NULL, NULL };
  Person * baby = new Person();
  Date * birth_date = new Date(Global::Sim_Date->get_year(day),
                               Global::Sim_Date->get_month(day),
                               Global::Sim_Date->get_day_of_month(day));
  baby->setup(id, age, sex, married, prof, favorite_place, pop, birth_date, false);
  baby->reset(Global::Sim_Date);
  delete birth_date;
  return baby;
}

void Person::register_event_handler(Person_Event_Interface *event_handler) {
  if(this->registered_event_handlers == NULL) {
    this->registered_event_handlers = new vector<Person_Event_Interface *>();
  }
  this->registered_event_handlers->push_back(event_handler);
}

void Person::deregister_event_handler(Person_Event_Interface *event_handler) {
  if (this->registered_event_handlers != NULL) {
    size_t vec_size = this->registered_event_handlers->size();
    size_t found_index = -1;
    bool found = false;
    for (size_t i = 0; i < vec_size && !found; i++) {
      if (this->registered_event_handlers->at(i) == event_handler) {
        found = true;
        found_index = i;
      }
    }
    if (found) {
      this->registered_event_handlers->
	erase(this->registered_event_handlers->begin() + found_index);
    }
  }
}

void Person::notify_property_change(string property_name, int prev_val, int new_val) {
  if (this->registered_event_handlers != NULL) {
    vector<Person_Event_Interface *>::iterator itr;
    for (itr = this->registered_event_handlers->begin();
	 itr < this->registered_event_handlers->end(); itr++ )
      (*itr)->handle_property_change_event(this, property_name, prev_val, new_val);
  }
}

void Person::notify_property_change(string property_name, bool new_val) {
  if (this->registered_event_handlers != NULL) {
    vector<Person_Event_Interface *>::iterator itr;
    for (itr = this->registered_event_handlers->begin();
	 itr < this->registered_event_handlers->end(); itr++ )
      (*itr)->handle_property_change_event(this, property_name, new_val);
  }
}

void Person::addIncidence(int disease, vector<int> strains) {
  activities->addIncidence(disease, strains);
}

void Person::addPrevalence(int disease, vector<int> strains) {
  activities->addPrevalence(disease, strains);
}

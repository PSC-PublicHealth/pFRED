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
#include "Date.h"
#include "Place_List.h"
#include "Person_Event_Interface.h"

#include <cstdio>
#include <vector>

Person::Person() { 
  idx = -1;
  pop = NULL;
  demographics = NULL;
  health = NULL;
  behavior = NULL;
  cognition = NULL;
  registered_event_handlers = NULL;
}

Person::~Person() {
  if (demographics != NULL) delete demographics;
  if (health != NULL) delete health;
  if (behavior != NULL) delete behavior;
  if (cognition != NULL) delete cognition;
  if (registered_event_handlers != NULL) delete registered_event_handlers;
}

void Person::setup(int index, int age, char sex, int marital,
                   int profession, Place **favorite_places, int profile,
                   Population* Pop, Date *sim_start_date, bool has_random_birthday)
{
  idx = index;
  pop = Pop;
  demographics = new Demographics(this, age, sex, marital, profession, sim_start_date, has_random_birthday);
  health = new Health(this);
  behavior = new Behavior(this, favorite_places, profile);
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
  
  fprintf(Tracefp,"\n");
  fflush(Tracefp);
}

void Person::print_out(int disease) const {
  if (disease < 0) printf("DEAD: ");
  disease = 0;
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
  
void Person::reset(Date * sim_start_date) {
  if (Verbose > 2) { fprintf(Statusfp, "reset person %d\n", idx); fflush(Statusfp); }
  demographics->reset(sim_start_date);
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

int Person::get_diseases() {
  return pop->get_diseases();
}

void Person::become_immune(Disease* disease) {
  int disease_id = disease->get_id();
  char status = health->get_disease_status(disease_id);
  if(status == 'S'){
    health->become_immune(disease);
  }
}

Place * Person::get_household() const {
  return behavior->get_household();
}

Place * Person::get_neighborhood() const {
  return behavior->get_neighborhood();
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

Person * Person::give_birth(int day) {

  int id = Population::get_next_id(), age = 0, married = -1, prof = 2;
  int school = -1, classroom = -1, work = -1;
  int office = -1, profile = 0;
  char sex = (URAND(0.0, 1.0) < 0.5 ? 'M' : 'F');

  Place *favorite_place[] = {
    this->get_household(),
    this->get_neighborhood(),
    Places.get_place(school),
    Places.get_place(classroom),
    Places.get_place(work),
    Places.get_place(office)
  };

  Person * baby = new Person();
  Date * birth_date = new Date(Sim_Start_Date->get_year(day),
                              Sim_Start_Date->get_month(day),
                              Sim_Start_Date->get_day_of_month(day));
  baby->setup(id, age, sex, married, prof, favorite_place, profile, this->pop, birth_date, false);
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
      this->registered_event_handlers->erase(this->registered_event_handlers->begin() + found_index);
    }
  }
}

void Person::notify_property_change(string property_name, int prev_val, int new_val) {

  if (this->registered_event_handlers != NULL) {
    vector<Person_Event_Interface *>::iterator itr;
    for (itr = this->registered_event_handlers->begin(); itr < this->registered_event_handlers->end(); itr++ )
      (*itr)->handle_property_change_event(this, property_name, prev_val, new_val);
  }
}

void Person::notify_property_change(string property_name, bool new_val) {

  if (this->registered_event_handlers != NULL) {
    vector<Person_Event_Interface *>::iterator itr;
    for (itr = this->registered_event_handlers->begin(); itr < this->registered_event_handlers->end(); itr++ )
      (*itr)->handle_property_change_event(this, property_name, new_val);
  }

}




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
#include "Transmission.h"

#include <cstdio>
#include <vector>
#include <sstream>

Person::Person() {
  idx = -1;
  demographics = NULL;
  health = NULL;
  activities = NULL;
  behavior = NULL;
}

Person::~Person() {
  if (demographics != NULL) delete demographics;
  if (health != NULL) delete health;
  if (activities != NULL) delete activities;
  if (behavior != NULL) delete behavior;
}

void Person::newborn_setup(int index, int age, char sex, int marital, int occ,
			   Place **favorite_places, int day) {
  idx = index;

  demographics = new Demographics(this, age, sex, marital, occ, day, true);
  health = new Health(this);
  activities = new Activities(this, favorite_places);
  behavior = new Behavior(this);
	
  for (int disease = 0; disease < Global::Diseases; disease++) {
    Disease* s = Global::Pop.get_disease(disease);
    if (!s->get_residual_immunity()->is_empty()) {
      double residual_immunity_prob =
	      s->get_residual_immunity()->find_value(get_age());
      if (RANDOM() < residual_immunity_prob)
	      become_immune(s);
    }
  }
}

Person::Person(int index, int age, char sex, int marital, int occ,
	       char *house, char *school, char *work, int day) {
  idx = index;
  demographics =
    new Demographics(this, age, sex, marital, occ, day, false);
  health = new Health(this);
  activities = new Activities(this, house, school, work);
  behavior = new Behavior(this);
	
  for (int disease = 0; disease < Global::Diseases; disease++) {
    Disease* s = Global::Pop.get_disease(disease);
    if (!s->get_residual_immunity()->is_empty()) {
      double residual_immunity_prob =
	s->get_residual_immunity()->find_value(get_age());
      if (RANDOM() < residual_immunity_prob)
	become_immune(s);
    }
  }
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

int Person::get_marital_status() const {
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

int Person::is_new_case(int day, int disease) const {
  return (health->get_exposure_date(disease) == day);
}

void Person::set_changed(){
  Global::Pop.set_changed(this);
}

void Person::infect(Person *infectee, int disease, Transmission *transmission) {
  this->health->infect(infectee, disease, transmission);
}

void Person::getInfected(Disease *disease, Transmission *transmission) {
  health->getInfected(disease, transmission);
  behavior->getInfected(disease, transmission);
}

Person * Person::give_birth(int day) {
  int id = Global::Pop.get_next_id(), age = 0, married = 0, prof = 2;
  char sex = (URAND(0.0, 1.0) < 0.5 ? 'M' : 'F');
  Place *favorite_place[] = {
    get_household(), get_neighborhood(),
    NULL, NULL, NULL, NULL };
  Person * baby = new Person();
  baby->newborn_setup(id, age, sex, married, prof, favorite_place, day);
  return baby;
}

void Person::addIncidence(int disease, vector<int> strains) {
  activities->addIncidence(disease, strains);
}

void Person::addPrevalence(int disease, vector<int> strains) {
  activities->addPrevalence(disease, strains);
}

string Person::to_string() {

  stringstream tmp_string_stream;
  // (i.e *ID* Age Sex Married Occupation Household School *Classroom* Workplace *Office*)
  tmp_string_stream << this->idx << " " << this->get_age() << " " <<  this->get_sex() << " " ;
  tmp_string_stream << this->get_marital_status() << " " << this->get_profession() << " ";
  Place *tmp_place = this->get_household();
  if(tmp_place == NULL)
    tmp_string_stream << "-1 ";
  else
    tmp_string_stream << tmp_place->get_label() << " ";

  tmp_place = this->get_school();
  if(tmp_place == NULL)
    tmp_string_stream << "-1 ";
  else
    tmp_string_stream << tmp_place->get_label() << " ";

  tmp_place = this->get_classroom();
  if(tmp_place == NULL)
    tmp_string_stream << "-1 ";
  else
    tmp_string_stream << tmp_place->get_label() << " ";

  tmp_place = this->get_workplace();
  if(tmp_place == NULL)
    tmp_string_stream << "-1 ";
  else
    tmp_string_stream << tmp_place->get_label() << " ";

  tmp_place = this->get_office();
  if(tmp_place == NULL)
    tmp_string_stream << "-1 ";
  else
    tmp_string_stream << tmp_place->get_label();

  return tmp_string_stream.str();
}

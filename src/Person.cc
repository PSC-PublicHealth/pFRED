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
#include "Perceptions.h"
#include "Place.h"
#include "Strain.h"
#include "Population.h"
//#include "Vaccine_Health.h"
#include "AV_Health.h"
#include "Age_Map.h"

#include <cstdio>

Person::Person() { 
  idx = -1;
  pop = NULL;
  demographics = NULL;
  health = NULL;
  behavior = NULL;
  perceptions = NULL;
}

Person::~Person() {
  delete demographics;
  delete health; 
  delete behavior;
  delete perceptions;
}

void Person::setup(int index, int age, char sex, int marital, int occ,
                   int profession, Place **favorite_places, int profile, Population* Pop) 
{
  idx = index;
  pop = Pop;
  demographics = new Demographics(this, age,sex,'U',marital,profession);
  health = new Health(this);
  behavior = new Behavior(this,favorite_places,profile);
  perceptions = new Perceptions(this);
}

void Person::print(int strain) const {
  fprintf(Tracefp, "%i %c id %7d  a %3d  s %c %c ",
          strain, health->get_strain_status(strain), idx,
          demographics->get_age(),
          demographics->get_sex(),
          demographics->get_occupation());
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
          health->get_exposure_date(strain), health->get_infectious_date(strain), health->get_recovered_date(strain));
  fprintf(Tracefp, "places %d ", behavior->get_favorite_places());
  fprintf(Tracefp, "infected_at %c %6d ",
          health->get_infected_place_type(strain), health->get_infected_place(strain));
  fprintf(Tracefp, "infector %d ", health->get_infector(strain));
  fprintf(Tracefp, "infector %d ", health->get_infector(strain));
  fprintf(Tracefp, "infectees %d ", health->get_infectees(strain));
  
  
  fprintf(Tracefp, "antivirals: %2d ",health->get_number_av_taken());
  for(int i=0;i<health->get_number_av_taken();i++)
    fprintf(Tracefp," %2d",health->get_av_health(i)->get_av_start_day());
  
  
  // fprintf(Tracefp, "vaccines: %2d", health->get_number_vaccines_taken());
  //   for(int i=0;i<health->get_number_vaccines_taken();i++)
  //     fprintf(Tracefp," %2d %2d %2d",health->get_vaccine_stat(i)->get_vaccination_day(),
  // 	    health->get_vaccine_stat(i)->is_effective(),health->get_vaccine_stat(i)->get_current_dose());
  fprintf(Tracefp,"\n");
  //   fflush(Tracefp);
}

void Person::print_out(int strain) const {
  fprintf(stdout, "%c id %7d  a %3d  s %c %c ",
          health->get_strain_status(strain), idx,
          demographics->get_age(),
          demographics->get_sex(),
          demographics->get_occupation());
  fprintf(stdout, "exp: %2d  inf: %2d  rem: %2d ",
          health->get_exposure_date(strain), health->get_infectious_date(strain), health->get_recovered_date(strain));
  fprintf(stdout, "places %d ", behavior->get_favorite_places());
  fprintf(stdout, "infected_at %c %6d ",
          health->get_infected_place_type(strain), health->get_infected_place(strain));
  fprintf(stdout, "infector %d ", health->get_infector(strain));
  fprintf(stdout, "infectees %d\n", health->get_infectees(strain));
  fflush(stdout);
}

void Person::print_schedule() const {
  behavior->print_schedule();
}

void Person::reset() {
	if (Verbose > 2) { fprintf(Statusfp, "reset person %d\n", idx); }
	demographics->reset();
	health->reset();
	perceptions->reset();
	behavior->reset();
	
	for (int strain = 0; strain < Pop.get_strains(); strain++) {
		Strain* s = Pop.get_strain(strain);
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
  perceptions->update(day);
  behavior->update(day);
}

void Person::become_susceptible(int strain) {
  health->become_susceptible(strain);
  behavior->become_susceptible(strain);
}

void Person::become_exposed(Infection * infection) {
  health->become_exposed(infection);
  behavior->become_exposed(infection->get_strain()->get_id());
}

void Person::become_infectious(Strain * strain) {
  int strain_id = strain->get_id();
  health->become_infectious(strain);
  behavior->become_infectious(strain_id);
}

void Person::become_symptomatic(Strain *strain) {
	health->become_symptomatic(strain);
}

void Person::become_immune(Strain* strain) {
  int strain_id = strain->get_id();
  health->become_immune(strain);
  behavior->become_immune(strain_id);
}

void Person::recover(Strain * strain) {
  int strain_id = strain->get_id();
  health->recover(strain);
  behavior->recover(strain_id);
  
  if (Verbose > 2) {
    fprintf(Statusfp, "RECOVERED person %d for strain %d\n", idx, strain_id);
    print_out(strain_id);
    fflush(Statusfp);
  }
}

void Person::update_schedule(int day) {
  return behavior->update_schedule(day);
}

void Person::get_schedule(int *n, Place **sched) const {
  behavior->get_schedule(n, sched);
}

void Person::behave(int day) {}

int Person::is_symptomatic() const {
  return health->is_symptomatic();
}

int Person::get_age() const {
  return demographics->get_age();
}

char Person::get_sex() const {
  return demographics->get_sex();
}

char Person::get_occupation() const {
  return demographics->get_occupation();
}

char Person::get_marital_status() const {
  return demographics->get_marital_status();
}

int Person::get_profession() const {
  return demographics->get_profession();
}

int Person::get_places() const {
  return behavior->get_favorite_places();
}

double Person::get_susceptibility(int strain) const {
  return health->get_susceptibility(strain);
}

double Person::get_infectivity(int strain) const {
  return health->get_infectivity(strain);
}

int Person::get_exposure_date(int strain) const {
  return health->get_exposure_date(strain);
}

int Person::get_infectious_date(int strain) const {
  return health->get_infectious_date(strain);
}

int Person::get_recovered_date(int strain) const {
  return health->get_recovered_date(strain);
}

int Person::get_infector(int strain) const {
  return health->get_infector(strain);
}

int Person::get_infected_place(int strain) const {
  return health->get_infected_place(strain);
}

char Person::get_infected_place_type(int strain) const {
  return health->get_infected_place_type(strain);
}

int Person::get_infectees(int strain) const {
  return health->get_infectees(strain);
}

int Person::add_infectee(int strain) {
  return health->add_infectee(strain);
}

int Person::is_new_case(int day, int strain) const {
  return (health->get_exposure_date(strain) == day);
}

void Person::set_changed(){
  this->pop->set_changed(this);
}
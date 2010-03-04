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

void Person::setup(int index, int age, char sex, int marital, int occ,
		   int profession, Place *house, Place *neigh,
		   Place *school, Place *classroom, Place *work,
		   Place *office, int profile) 
{
  id = index;
  demographics = new Demographics(age,sex,'U',marital,profession);
  health = new Health(this);
  behavior = new Behavior(this,house,neigh,school,classroom,work,office,profile);
  perceptions = new Perceptions(this);
}
  
void Person::print(int strain) {
  fprintf(Tracefp, "%i %c id %7d  a %3d  s %c %c ",
	  strain, health->get_strain_status(strain), id,
	  demographics->get_age(),
	  demographics->get_sex(),
	  demographics->get_occupation());
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
	  health->get_exposure_date(strain), health->get_infectious_date(strain), health->get_recovered_date(strain));
  fprintf(Tracefp, "places %d ", behavior->get_favorite_places());
  fprintf(Tracefp, "infected_at %c %6d ",
	  health->get_infected_place_type(strain), health->get_infected_place(strain));
  fprintf(Tracefp, "infector %d ", health->get_infector(strain));
  fprintf(Tracefp, "infectees %d ", health->get_infectees(strain));
  fprintf(Tracefp, "antivirals: %2d ",health->get_number_av_taken());
  for(int i=0;i<health->get_number_av_taken();i++)
    fprintf(Tracefp," %2d",health->get_antiviral_start_date(i));
  
  fprintf(Tracefp,"\n");
  fflush(Tracefp);
}

void Person::print_out(int strain) {
  fprintf(stdout, "%c id %7d  a %3d  s %c %c ",
	  health->get_strain_status(strain), id,
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

void Person::print_schedule() {
  behavior->print_schedule();
}
  
void Person::reset() {
  if (Verbose > 2) { fprintf(Statusfp, "reset person %d\n", id); }
  demographics->reset();
  health->reset();
  perceptions->reset();
  behavior->reset();
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

void Person::recover(Strain * strain) {
  int strain_id = strain->get_id();
  health->recover(strain);
  behavior->recover(strain_id);

  // print recovered agents into Trace file
  print(strain_id);
  if (Verbose > 2) {
    fprintf(Statusfp, "RECOVERED person %d for strain %d\n", id, strain_id);
    print_out(strain_id);
    fflush(Statusfp);
  }
}

void Person::update_schedule(int day) {
  return behavior->update_schedule(day);
}

void Person::get_schedule(int *n, int *sched) {
  behavior->get_schedule(n, sched);
}

void Person::behave(int day) {}

int Person::is_symptomatic() {
  return health->is_symptomatic();
}

const Antiviral* Person::get_av(int strain, int day) {
  return health->get_av(strain, day);
}

int Person::get_age() {
  return demographics->get_age();
}

char Person::get_sex() {
  return demographics->get_sex();
}

char Person::get_occupation() {
  return demographics->get_occupation();
}

char Person::get_marital_status() {
  return demographics->get_marital_status();
}

int Person::get_profession() {
  return demographics->get_profession();
}

int Person::get_places() {
  return behavior->get_favorite_places();
}

double Person::get_susceptibility(int strain) {
  return health->get_susceptibility(strain);
}

double Person::get_infectivity(int strain) {
  return health->get_infectivity(strain);
}

int Person::get_exposure_date(int strain) {
  return health->get_exposure_date(strain);
}

int Person::get_infectious_date(int strain) {
  return health->get_infectious_date(strain);
}

int Person::get_recovered_date(int strain) {
  return health->get_recovered_date(strain);
}

int Person::get_infector(int strain) {
  return health->get_infector(strain);
}

int Person::get_infected_place(int strain) {
  return health->get_infected_place(strain);
}

char Person::get_infected_place_type(int strain) {
  return health->get_infected_place_type(strain);
}

int Person::get_infectees(int strain) {
  return health->get_infectees(strain);
}

int Person::add_infectee(int strain) {
  return health->add_infectee(strain);
}

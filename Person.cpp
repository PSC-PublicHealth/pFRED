/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Person.cpp
//

#include "Person.hpp";
#include "Global.hpp"
#include "Profile.hpp"
#include "Population.hpp"
#include "Strain.hpp"
#include "Random.hpp"
#include "Place.hpp"
#include "Demographics.hpp";
#include "Behavior.hpp"
#include "Health.hpp"

void Person::setup(int i, int a, char g, int m, int o, int p, Place *h,
		   Place *n, Place *s, Place *c, Place *w, Place *off, int pro) 
{
  id = i;
  demographics = new Demographics(a,g,'U',m,p);
  behavior = new Behavior(h,n,s,c,w,off,pro);
  health = new Health(id);
}
  
void Person::print(int strain) {
  fprintf(Tracefp, "%c id %7d  a %3d  s %c %c ",
	  health->get_strain_status(strain), id,
	  demographics->get_age(),
	  demographics->get_sex(),
	  demographics->get_occupation());
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
	  health->get_exposure_date(strain), health->get_infectious_date(strain), health->get_recovered_date(strain));
  fprintf(Tracefp, "places %d ", behavior->get_favorite_places());
  fprintf(Tracefp, "infected_at %c %6d ",
	  health->get_infected_place_type(strain), health->get_infected_place(strain));
  fprintf(Tracefp, "infector %d ", health->get_infector(strain));
  fprintf(Tracefp, "infectees %d\n", health->get_infectees(strain));
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
  behavior->print_schedule(id);
}
  
void Person::make_susceptible() {
  if (Verbose > 2) { fprintf(Statusfp, "SUSCEPTIBLE person %d\n", id); }
  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    health->make_susceptible(id, s);
    behavior->make_susceptible(id, s);
  }
}

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

void Person::make_exposed(int strain, int per, int place, char type, int day) {
  health->make_exposed(id, strain, per, place, type, day);
  Pop.insert_into_exposed_list(strain, id);
}
  
void Person::make_infectious(int strain) {
  health->make_infectious(id, strain);
  behavior->make_infectious(id, strain, health->get_exposure_date(strain));
  Pop.remove_from_exposed_list(strain, id);
  Pop.insert_into_infectious_list(strain, id);
}

void Person::make_recovered(int strain) {
  health->make_recovered(id, strain);
  if (Verbose > 2) {
    fprintf(Statusfp, "RECOVERED person %d for strain %d\n", id, strain);
    print_out(strain);
    fflush(Statusfp);
  }
  behavior->make_recovered(id, strain, health->get_exposure_date(strain));
  Pop.remove_from_infectious_list(strain, id);

  // print recovered agents into Trace file
  print(strain);
}

int Person::is_on_schedule(int day, int loc) {
  return behavior->is_on_schedule(id, day, loc, is_symptomatic());
}

void Person::update_schedule(int day) {
  return behavior->update_schedule(id, day, is_symptomatic());
}

void Person::get_schedule(int *n, int *sched) {
  behavior->get_schedule(n, sched);
}

void Person::behave(int day) {}

int Person::is_symptomatic() {
  return health->is_symptomatic();
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

char Person::get_strain_status(int strain) {
  return health->get_strain_status(strain);
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

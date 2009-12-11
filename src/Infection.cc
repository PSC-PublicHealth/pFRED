/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Infection.cc
//

#include "Infection.h"

// For logging purposes.
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Strain.h"

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

Infection::Infection (Strain * s, Person* person_infector,
		      Person* infectee, Place* place, int day) {
  strain = s;
  strain_status = 'E';
  exposure_date = day;
  latent_period = strain->get_days_latent();
  infectious_period = strain->get_days_infectious();
  infectious_date = exposure_date + latent_period;
  will_be_symptomatic = strain->get_symptoms();
  recovered_date = infectious_date + infectious_period;
  infector = person_infector;
  infected_place = place;
  host = infectee;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  infectees = 0;
}

// This logic shouldn't be here - we should have a dummy infected place
// for seed infections.  We could call it the seed farm.  That would be
// funny.
int Infection::get_infected_place_id() {
  if (infected_place != NULL) {
    return infected_place->get_id();
  }
  return -1;
}

char Infection::get_infected_place_type() {
  if (infected_place != NULL) {
    return infected_place->get_type();
  }
  return 'X';
}

int Infection::get_infector() {
  if (infector != NULL) {
    return infector->get_id();
  }
  return -1;
}

void Infection::become_infectious() {
  if (will_be_symptomatic) {
    strain_status = 'I';
    infectivity = 1.0;
    symptoms = 1.0;
  }
  else {
    strain_status = 'i';
    infectivity = 0.5;
    symptoms = 0.0;
  }
}

void Infection::recover() {
  strain_status = 'R';
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
}

void Infection::reset_infection_course(int num_latent_days, int num_infectious_days,
				       bool will_have_symptoms, int current_day) {
  // If my host has already recovered, changing any parameters is invalid.
  if (current_day > recovered_date) {
    printf("Infection from strain %i: attempt to change infection course after"
	   " person %i is already recovered.  Recovery date was %i, and current "
	   "date is %i\n",
	   strain->get_id(), host->get_id(), recovered_date, current_day);
    fflush(stdout);
    abort();
  }
  // If my host is already infectious, changing the infectious date is invalid.
  if (current_day > infectious_date && latent_period != num_latent_days) {
    printf("Infection from strain %i: attempt to change latent period from % i "
	   "to %i after person %i is already infectious.  Current infectious "
	   "date is %i, new day would be %i\n",
	   strain->get_id(), latent_period, num_latent_days, host->get_id(),
	   infectious_date, exposure_date + num_latent_days);
    fflush(stdout);
    abort();
  }

  latent_period = num_latent_days;
  infectious_period = num_infectious_days;
  infectious_date = exposure_date + latent_period;
  will_be_symptomatic = will_have_symptoms;
  recovered_date = infectious_date + infectious_period;
  if (Verbose > 2) {
    fprintf(Statusfp, "reset strain - new infectious_date: %i new recovered date: %i\n",
	    infectious_date, recovered_date);
    fflush(Statusfp);
  }
}

bool Infection::possibly_mutate(int day) {
  if (day > recovered_date) {
    printf("Cannot mutate a recovered infection "
	   "strain %i person %i on date %i status is %c recovered date is %i\n",
	   strain->get_id(), host->get_id(), day, strain_status, recovered_date);
    fflush(stdout);
    abort();
  }
  Strain* new_strain = strain->should_mutate_to();
  if (new_strain == NULL) {
    return false;
  }
  else {
    if (Verbose) {
      fprintf(Statusfp,"Person %i will mutate from strain %i to strain %i \n", host->get_id(),
	      strain->get_id(), new_strain->get_id());
      fflush(Statusfp);
    }
    Infection* new_infection = new Infection(new_strain, host, host, NULL, day);
    // Reset the new infection to take this infection's place
    printf("person %i new strain's course: latent %i recovered %i \n", host->get_id(),
	   0, recovered_date - day);
    new_infection->reset_infection_course(0, recovered_date - day, will_be_symptomatic, day);

    host->become_exposed(new_infection);
    host->add_infectee(new_strain->get_id());

    // Update this infection so that we recover today.
    infectious_period = day - infectious_date;
    infectious_date = day;
    recovered_date = day;
    return true;
  }
}


// static 
Infection* Infection::get_dummy_infection(Strain *s, Person* host, int day) {
  Infection* i = new Infection(s, NULL, host, NULL, day);
  i->reset_infection_course(0, 0, false, day);
  return i;
}

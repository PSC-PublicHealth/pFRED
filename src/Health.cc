/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.cc
//

#include <new>
#include "Health.h"
#include "Person.h"
#include "Strain.h"
#include "Infection.h"
#include "Population.h"
#include "Global.h"

Health::Health (Person * person) {
  self = person;
  strains = Pop.get_strains();
  infection = new vector <Infection *> [strains];
  reset();
}

void Health::reset() {
  for (int strain = 0; strain < strains; strain++) {
    infection[strain].clear();
  }
}

void Health::update(int day) {
  for (int s = 0; s < strains; s++) {
    char status = get_strain_status(s);
    if (status == 'S')
      continue;
    Strain * strain = infection[s][0]->get_strain();
    if (status == 'E') {
      if (day == get_infectious_date(s)) {
	self->become_infectious(strain);
      }
    }
    status = get_strain_status(s);
    if (status == 'I' || status == 'i') {
      if (day == get_recovered_date(s)) {
	self->recover(strain);
      }
    }
  }
}

void Health::become_exposed(Infection * infection_ptr) {
  Strain * strain = infection_ptr->get_strain();
  int strain_id = strain->get_id();
  if (Verbose > 1) {
    fprintf(Statusfp, "EXPOSED person %d to strain %d\n", self->get_id(), strain_id);
  }
  infection[strain_id].push_back(infection_ptr);
  strain->insert_into_exposed_list(self);
}

void Health::become_infectious(Strain * strain) {
  int strain_id = strain->get_id();
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", self->get_id(), strain_id);
    fflush(Statusfp);
  }
  infection[strain_id][0]->become_infectious();
  strain->remove_from_exposed_list(self);
  strain->insert_into_infectious_list(self);
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d has status %c\n",
	    self->get_id(), strain_id, get_strain_status(strain_id));
    fflush(Statusfp);
  }
}

void Health::recover(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id][0]->recover();
  strain->remove_from_infectious_list(self);
}

int Health::is_symptomatic() {
  for (int strain = 0; strain < strains; strain++) {
    if (!infection[strain].empty() && infection[strain][0]->is_symptomatic())
      return 1;
  }
  return 0;
}

int Health::get_exposure_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_exposure_date();
}

int Health::get_infectious_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infectious_date();
}

int Health::get_recovered_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_recovered_date();
}

int Health::get_infector(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infector();
}

int Health::get_infected_place(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infected_place();
}

char Health::get_infected_place_type(int strain) {
  if (infection[strain].empty()) 
    return 'X';
  else
    return infection[strain][0]->get_infected_place_type();
}

int Health::get_infectees(int strain) {
  if (infection[strain].empty()) 
    return 0;
  else
    return infection[strain][0]->get_infectees();
}

int Health::add_infectee(int strain) {
  if (infection[strain].empty())
    return 0;
  else
    return infection[strain][0]->add_infectee();
}

char Health::get_strain_status(int strain) {
  if (infection[strain].empty())
    return 'S';
  else
    return infection[strain][0]->get_strain_status();
}

double Health::get_susceptibility(int strain) {
  if (infection[strain].empty())
    return 1.0;
  else
    return infection[strain][0]->get_susceptibility();
}

double Health::get_infectivity(int strain) {
  if (infection[strain].empty())
    return 0.0;
  else
    return infection[strain][0]->get_infectivity();
}

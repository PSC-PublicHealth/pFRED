/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.cpp
//

#include <new>
#include "Person.hpp"
#include "Strain.hpp"
#include "Infection.hpp"
#include "Global.hpp"

Person::Health::Health (Person * person) {
  me = person;
  id = me->get_id();;
  int strains = Pop.get_strains();
  infection = new vector <Infection *> [strains];
  reset();
}

void Person::Health::reset() {
  int strains = Pop.get_strains();
  for (int strain = 0; strain < strains; strain++) {
    infection[strain].clear();
  }
}

void Person::Health::become_exposed(Strain * strain, int infector, int place, char type, int day) {
  int strain_id = strain->get_id();
  infection[strain_id].push_back(new Infection(id, strain, infector, place, type, day));
  strain->insert_into_exposed_list(me);
}

void Person::Health::become_infectious(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id][0]->become_infectious(id, strain);
  strain->remove_from_exposed_list(me);
  strain->insert_into_infectious_list(me);
}

void Person::Health::recover(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id][0]->recover(id, strain);
  strain->remove_from_infectious_list(me);
}

int Person::Health::is_symptomatic() {
  int strains = Pop.get_strains();
  for (int strain = 0; strain < strains; strain++) {
    if (!infection[strain].empty() && infection[strain][0]->is_symptomatic())
      return 1;
  }
  return 0;
}

int Person::Health::get_exposure_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_exposure_date();
}

int Person::Health::get_infectious_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infectious_date();
}

int Person::Health::get_recovered_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_recovered_date();
}

int Person::Health::get_infector(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infector();
}

int Person::Health::get_infected_place(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infected_place();
}

char Person::Health::get_infected_place_type(int strain) {
  if (infection[strain].empty()) 
    return 'X';
  else
    return infection[strain][0]->get_infected_place_type();
}

int Person::Health::get_infectees(int strain) {
  if (infection[strain].empty()) 
    return 0;
  else
    return infection[strain][0]->get_infectees();
}

int Person::Health::add_infectee(int strain) {
  if (infection[strain].empty())
    return 0;
  else
    return infection[strain][0]->add_infectee();
}

char Person::Health::get_strain_status(int strain) {
  if (infection[strain].empty())
    return 'S';
  else
    return infection[strain][0]->get_strain_status();
}

double Person::Health::get_susceptibility(int strain) {
  if (infection[strain].empty())
    return 1.0;
  else
    return infection[strain][0]->get_susceptibility();
}

double Person::Health::get_infectivity(int strain) {
  if (infection[strain].empty())
    return 0.0;
  else
    return infection[strain][0]->get_infectivity();
}

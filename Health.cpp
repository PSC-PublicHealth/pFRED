/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.cpp
//

#include <stdio.h>
#include <new>
#include "Health.hpp"
#include "Strain.hpp"
#include "Infection.hpp"

Health::Health () {
  int strains = Strain::get_strains();
  infection = new vector <Infection *> [strains];
  reset();
}

void Health::reset() {
  int strains = Strain::get_strains();
  for (int strain = 0; strain < strains; strain++) {
    infection[strain].clear();
  }
  printf("health reset done\n"); fflush(stdout);
}

void Health::become_exposed(int id, int strain, int per, int place, char type, int day) {
  infection[strain].push_back(new Infection(id, strain, per, place, type, day));
}

void Health::become_infectious(int id, int strain) {
  infection[strain][0]->become_infectious(id, strain);
}

void Health::recover(int id, int strain) {
  infection[strain][0]->recover(id, strain);
}

int Health::is_symptomatic() {
  int strains = Strain::get_strains();
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

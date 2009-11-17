/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.cpp
//

#include "Health.hpp"
#include "Global.hpp"
#include "Random.hpp"
#include "Strain.hpp"

Health::Health (int id) {
  int strains = Strain::get_strains();
  strain_status = new (nothrow) char [strains];
  if (strain_status == NULL) {
    printf("Help! strain_status allocation failure for Person %d\n", id);
    abort();
  }

  latent_period = new (nothrow) int [strains];
  if (latent_period == NULL) {
    printf("Help! latent_period allocation failure for Person %d\n", id);
    abort();
  }

  infectious_period = new (nothrow) int [strains];
  if (infectious_period == NULL) {
    printf("Help! infectious_period allocation failure for Person %d\n", id);
    abort();
  }

  exposure_date = new (nothrow) int [strains];
  if (exposure_date == NULL) {
    printf("Help! exposure_date allocation failure for Person %d\n", id);
    abort();
  }

  infectious_date = new (nothrow) int [strains];
  if (infectious_date == NULL) {
    printf("Help! infectious_date allocation failure for Person %d\n", id);
    abort();
  }

  recovered_date = new (nothrow) int [strains];
  if (recovered_date == NULL) {
    printf("Help! recovered_date allocation failure for Person %d\n", id);
    abort();
  }

  infector = new (nothrow) int [strains];
  if (infector == NULL) {
    printf("Help! infector allocation failure for Person %d\n", id);
    abort();
  }

  infected_place = new (nothrow) int [strains];
  if (infected_place == NULL) {
    printf("Help! infected_place allocation failure for Person %d\n", id);
    abort();
  }

  infected_place_type = new (nothrow) char [strains];
  if (infected_place_type == NULL) {
    printf("Help! infected_place_type allocation failure for Person %d\n", id);
    abort();
  }

  infectees = new (nothrow) int [strains];
  if (infectees == NULL) {
    printf("Help! infectees allocation failure for Person %d\n", id);
    abort();
  }

  susceptibility = new (nothrow) double [strains];
  if (susceptibility == NULL) {
    printf("Help! susceptibility allocation failure for Person %d\n", id);
    abort();
  }

  infectivity = new (nothrow) double [strains];
  if (infectivity == NULL) {
    printf("Help! infectivity allocation failure for Person %d\n", id);
    abort();
  }

  for (int s = 0; s < strains; s++) {
    infected_place[s] = -1;
    infected_place_type[s] = 'X';
  }
  // printf("setup complete for person %d",id); fflush(stdout);
}

void Health::make_susceptible(int id, int strain) {
  strain_status[strain] = 'S';
  exposure_date[strain] = infectious_date[strain] = recovered_date[strain] = -1;
  infected_place[strain] = -1;
  infected_place_type[strain] = 'X';
  infector[strain] = -1;
  infectees[strain] = 0;
  susceptibility[strain] = 1.0;
  infectivity[strain] = 0.0;
}

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

void Health::make_exposed(int id, int strain, int per, int place, char type, int day) {
  if (Verbose > 1) { fprintf(Statusfp, "EXPOSED person %d\n", id); }
  strain_status[strain] = 'E';
  exposure_date[strain] = day;
  latent_period[strain] = Strain::get_days_latent(strain);
  infectious_period[strain] = Strain::get_days_infectious(strain);
  infectious_date[strain] = exposure_date[strain] + latent_period[strain];
  recovered_date[strain] = infectious_date[strain] + infectious_period[strain];
  infector[strain] = per;
  infected_place[strain] = place;
  if (place == -1) { 
    infected_place_type[strain] = 'X';
  }
  else {
    infected_place_type[strain] = type;
  }
  susceptibility[strain] = 0.0;
  // if (Verbose > 2) { print_out(strain); }
}

void Health::make_infectious(int id, int strain) {
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", id, strain);
    fflush(Statusfp);
  }
  if (RANDOM() < Strain::get_prob_symptomatic(strain)) {
    strain_status[strain] = 'I';
    infectivity[strain] = 1.0;
  }
  else {
    strain_status[strain] = 'i';
    infectivity[strain] = 0.5;
  }
}

void Health::make_recovered(int id, int strain) {
  strain_status[strain] = 'R';
}

int Health::is_symptomatic() {
  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    if (strain_status[s] == 'I')
      return 1;
  }
  return 0;
}


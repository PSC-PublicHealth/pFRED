/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Infection.cpp
//

#include "Infection.hpp"
#include "Global.hpp"
#include "Random.hpp"
#include "Strain.hpp"

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

Infection::Infection (int id, Strain * strain, int per, int place, char type, int day) {
  if (Verbose > 1) { fprintf(Statusfp, "EXPOSED person %d\n", id); }
  strain_status = 'E';
  exposure_date = day;
  latent_period = strain->get_days_latent();
  infectious_period = strain->get_days_infectious();
  infectious_date = exposure_date + latent_period;
  recovered_date = infectious_date + infectious_period;
  infector = per;
  infected_place = place;
  if (place == -1) { 
    infected_place_type = 'X';
  }
  else {
    infected_place_type = type;
  }
  susceptibility = 0.0;
  infectees = 0;
}

void Infection::become_infectious(int id, Strain * strain) {
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", id, strain->get_id());
    fflush(Statusfp);
  }
  if (RANDOM() < strain->get_prob_symptomatic()) {
    strain_status = 'I';
    infectivity = 1.0;
  }
  else {
    strain_status = 'i';
    infectivity = 0.5;
  }
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d has status %c\n", id, strain->get_id(), strain_status);
    fflush(Statusfp);
  }
}

void Infection::recover(int id, Strain * strain) {
  strain_status = 'R';
}

int Infection::is_symptomatic() {
  if (strain_status == 'I')
    return 1;
  else
    return 0;
}


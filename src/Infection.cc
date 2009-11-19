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
#include "Random.h"
#include "Strain.h"

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

Infection::Infection (Strain * s, int person, int place, char type, int day) {
  strain = s;
  strain_status = 'E';
  exposure_date = day;
  latent_period = strain->get_days_latent();
  infectious_period = strain->get_days_infectious();
  infectious_date = exposure_date + latent_period;
  will_be_symptomatic = strain->get_symptoms();
  recovered_date = infectious_date + infectious_period;
  infector = person;
  infected_place = place;
  if (place == -1) { 
    infected_place_type = 'X';
  }
  else {
    infected_place_type = type;
  }
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  infectees = 0;
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


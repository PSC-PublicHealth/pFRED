/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Perceptions.cc
//

#include "Perceptions.h"
#include "Person.h"
#include "Spread.h"
#include "Strain.h"
#include "Household.h"

int Perceptions::get_neighborhood_cases(int strain) {
  Place * p = self->get_behavior()->get_neighborhood();
  if (p == NULL) return 0.0;
  else return p->get_daily_cases(strain);
}

int Perceptions::get_neighborhood_deaths(int strain) {
  Place * p = self->get_behavior()->get_neighborhood();
  if (p == NULL) return 0.0;
  else return p->get_daily_deaths(strain);
}

int Perceptions::get_workplace_cases(int strain) {
  int count = 0;
  Place * p = self->get_behavior()->get_workplace();
  if (p != NULL) count = p->get_daily_cases(strain);
  return count;
}

int Perceptions::get_workplace_deaths(int strain) {
  int count = 0;
  Place * p = self->get_behavior()->get_workplace();
  if (p != NULL) count = p->get_daily_deaths(strain);
  return count;
}

int Perceptions::get_school_cases(int strain) {
  int count = 0;
  Place * p = self->get_behavior()->get_school();
  if (p != NULL) count = p->get_daily_cases(strain);
  return count;
}

int Perceptions::get_school_deaths(int strain) {
  int count = 0;
  Place * p = self->get_behavior()->get_school();
  if (p != NULL) count = p->get_daily_deaths(strain);
  return count;
}

double Perceptions::get_household_school_incidence(int strain) {
  int count = 0;
  int total_school = 0;
  Household * house = (Household *) self->get_behavior()->get_household();
  int hsize = house->get_size();
  for (int i = 0; i < hsize; i++) {
    Person * pp = house->get_housemate(i);
    Place * s = pp->get_behavior()->get_school();
    if (s != NULL) { 
      count += s->get_daily_cases(strain);
      total_school += s->get_size();
    }
  }
  if (total_school == 0) return 0.0;
  return (double) count / (double) total_school;
}



/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Perceptions.h
//

#ifndef _FRED_PERCEPTIONS_H
#define _FRED_PERCEPTIONS_H

#include "Place.h"
#include "Person.h"
#include "Spread.h"
#include "Strain.h"
#include "Household.h"

class Perceptions {
public:
  Perceptions(Person *p) { self = p; }
  virtual void reset() {}
  virtual void update(int day) {}
  virtual int get_global_cases(int strain) {
    return self->get_population()->get_strain(strain)->get_spread()->get_clinical_incidents();
  }
  virtual int get_global_deaths(int strain) {
    return self->get_population()->get_strain(strain)->get_mortality_rate()*get_global_cases(strain);
  }
  virtual int get_neighborhood_cases(int strain);
  virtual int get_neighborhood_deaths(int strain);
  virtual int get_workplace_cases(int strain);
  virtual int get_workplace_deaths(int strain);
  virtual int get_school_cases(int strain);
  virtual int get_school_deaths(int strain);
  virtual double get_household_school_incidence(int strain);
  virtual int get_local_cases(int strain) { return 0.0; }
  virtual int get_local_deaths(int strain) { return 0.0; }
private:
  Person * self;
protected:
  Perceptions() {}
  ~Perceptions() {}
};

#endif // _FRED_PERCEPTIONS_H


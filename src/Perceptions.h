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

#include "Global.h"
#include "Place.h"
#include "Person.h"
#include "Disease.h"
#include "Epidemic.h"
#include "Household.h"

class Perceptions {
public:
  Perceptions(Person *p) { self = p; }
  UNIT_TEST_VIRTUAL void reset() {}
  UNIT_TEST_VIRTUAL void update(int day) {}
  UNIT_TEST_VIRTUAL int get_global_cases(int disease) {
    return self->get_population()->get_disease(disease)->get_epidemic()->get_clinical_incidents();
  }
  UNIT_TEST_VIRTUAL int get_global_deaths(int disease) {
    return self->get_population()->get_disease(disease)->get_mortality_rate()*get_global_cases(disease);
  }
  UNIT_TEST_VIRTUAL int get_neighborhood_cases(int disease);
  UNIT_TEST_VIRTUAL int get_neighborhood_deaths(int disease);
  UNIT_TEST_VIRTUAL int get_workplace_cases(int disease);
  UNIT_TEST_VIRTUAL int get_workplace_deaths(int disease);
  UNIT_TEST_VIRTUAL int get_school_cases(int disease);
  UNIT_TEST_VIRTUAL int get_school_deaths(int disease);
  UNIT_TEST_VIRTUAL double get_household_school_incidence(int disease);
  UNIT_TEST_VIRTUAL int get_local_cases(int disease) { return 0.0; }
  UNIT_TEST_VIRTUAL int get_local_deaths(int disease) { return 0.0; }
private:
  Person * self;
protected:
  Perceptions() {}
  ~Perceptions() {}
};

#endif // _FRED_PERCEPTIONS_H


/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Perceptions.h
//

#ifndef _FRED_PERCEPTIONS_H
#define _FRED_PERCEPTIONS_H

#include "Global.h"
#include "Population.h"
#include "Disease.h"
#include "Epidemic.h"
class Person;

class Perceptions {
public:

  /**
   * Construct a new Persceptions with a pointer to the Person to which it belongs.
   * @param p a pointer to a Person
   */
  Perceptions(Person *p) { self = p; }

  /**
   * Perform the daily update for this object
   *
   * @param day the simulation day
   */
  void update(int day) {}

  /**
   * @return the number of deaths from a given disease throughout the Population
   */
  int get_global_deaths(int disease) {
    // return Global::Pop.get_disease(disease)->get_mortality_rate()*get_global_cases(disease);
    return 0.0;
  }

  /**
   * @params disease the disease in question
   * @return the count of cases of the given disease in this person's neighborhood
   */
  int get_neighborhood_cases(int disease);

  /**
   * @params disease the disease in question
   * @return the count of deaths from the given disease in this person's neighborhood
   */
  int get_neighborhood_deaths(int disease);

  /**
   * @params disease the disease in question
   * @return the count of cases of the given disease in this person's workplace
   */
  int get_workplace_cases(int disease);

  /**
   * @params disease the disease in question
   * @return the count of deaths from the given disease in this person's workplace
   */
  int get_workplace_deaths(int disease);

  /**
   * @params disease the disease in question
   * @return the count of cases of the given disease in this person's school
   */
  int get_school_cases(int disease);

  /**
   * @params disease the disease in question
   * @return the count of deaths from the given disease in this person's school
   */
  int get_school_deaths(int disease);

  /**
   * @params disease the disease in question
   * @return the incidence rate of the given disease in this person's household and school
   */
  double get_household_school_incidence(int disease);

  /**
   * Not currently implemented
   * @params disease the disease in question
   * @return the count of cases of the given disease locally (0)
   */
  int get_local_cases(int disease) { return 0.0; }

  /**
   * Not currently implemented
   * @params disease the disease in question
   * @return the count of deaths from the given disease locally (0)
   */
  int get_local_deaths(int disease) { return 0.0; }
private:
  Person * self;
protected:
  Perceptions() {}
  ~Perceptions() {}
};

#endif // _FRED_PERCEPTIONS_H


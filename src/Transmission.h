/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
// File: Transmission.h
//

#ifndef _FRED_TRANSMISSION_H
#define _FRED_TRANSMISSION_H

#include "Global.h"
#include <map>

class Person;
class Place;

class Transmission {
  public:
    // if primary transmission, infector and place are null.
    // if mutation, place is null.
    Transmission(Person *infector, Place* place, int day) : infector(infector), place(place), exposure_date(day) { }
    UNIT_TEST_VIRTUAL ~Transmission();

    // general
    UNIT_TEST_VIRTUAL Person *getInfector() const {
      return infector;
      }
    UNIT_TEST_VIRTUAL Place *getInfectedPlace() const {
      return place;
      }
    UNIT_TEST_VIRTUAL void setInitialLoads(std::map<int, double> *initialLoads) {
      this->initialLoads = initialLoads;
      }
    UNIT_TEST_VIRTUAL std::map<int, double> * getInitialLoads() {
      return initialLoads;
      }
    UNIT_TEST_VIRTUAL void print() const;

    // chrono
    UNIT_TEST_VIRTUAL int get_exposure_date() const {
      return exposure_date;
      }

  private:
    Person *infector;
    Place *place;
    int exposure_date;

    std::map<int, double> *initialLoads;
  };

#endif // _FRED_TRANSMISSION_H

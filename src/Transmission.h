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
    Transmission(Person *infector, Place *place, int day) : infector(infector), place(place), exposure_date(day) { }
    ~Transmission() {
      initialLoads->clear();
      delete initialLoads;
    }

    // general
    /**
     * @return a pointer to the Person object that is the infector
     */
    Person * getInfector() const {
      return infector;
    }

    /**
     * @return a pointer to the Place object where the Transmission occured
     */
    Place * getInfectedPlace() const {
      return place;
    }

    /**
     * @param initialLoads the new initialLoads
     */
    void setInitialLoads(std::map<int, double> *initialLoads) {
      this->initialLoads = initialLoads;
    }

    /**
     * @return the map of initial loads for this Transmission
     */
    std::map<int, double> * getInitialLoads() {
      return initialLoads;
    }

    /**
     * Print out information about this object
     */
    void print() const;

    // chrono
    /**
     * @return the exposure_date
     */
    int get_exposure_date() const {
      return exposure_date;
    }

  private:
    Person *infector;
    Place *place;
    int exposure_date;

    std::map<int, double> *initialLoads;
  };

#endif // _FRED_TRANSMISSION_H

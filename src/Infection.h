/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
// File: Infection.h
//

#ifndef _FRED_INFECTION_H
#define _FRED_INFECTION_H

#include "Global.h"
#include <map>
#include "Trajectory.h"
#include "IntraHost.h"
#include <limits.h>

class Health;
class Person;
class Place;
class Disease;
class Antiviral;
class Health;
class IntraHost;
class Transmission;

#define BIFURCATING 0
#define SEQUENTIAL 1

extern int Infection_model;

class Infection {
public:
  // if primary infection, infector and place are null.
  // if mutation, place is null.
  Infection(Disease *s, Person *infector, Person *infectee, Place* place, int day);
  UNIT_TEST_VIRTUAL ~Infection() { }

  // infection state
  UNIT_TEST_VIRTUAL char get_disease_status() const {
    return status;
  }
  UNIT_TEST_VIRTUAL void become_infectious();
  UNIT_TEST_VIRTUAL void become_symptomatic();
  UNIT_TEST_VIRTUAL void become_susceptible();
  UNIT_TEST_VIRTUAL void become_unsusceptible();
  UNIT_TEST_VIRTUAL void recover();
  UNIT_TEST_VIRTUAL void remove();
  UNIT_TEST_VIRTUAL void update(int today);
  bool is_infectious() const { return (status == 'I' || status == 'i'); }

  // general
  UNIT_TEST_VIRTUAL Disease *get_disease() const { return disease; }
  UNIT_TEST_VIRTUAL Person *get_infector() const { return infector; }
  UNIT_TEST_VIRTUAL Place *get_infected_place() const { return place; }
  UNIT_TEST_VIRTUAL int get_infectee_count() const { return infectee_count; }
  UNIT_TEST_VIRTUAL int add_infectee() { return ++infectee_count; }
  UNIT_TEST_VIRTUAL void print() const;
  void report_infection(int day) const;

  // chrono
  UNIT_TEST_VIRTUAL int get_exposure_date() const { return exposure_date; }
  UNIT_TEST_VIRTUAL int get_infectious_date() const { return infectious_date - offset; }
  UNIT_TEST_VIRTUAL int get_symptomatic_date() const { return symptomatic_date - offset; }
  int get_asymptomatic_date() const { return asymptomatic_date - offset; }
  UNIT_TEST_VIRTUAL int get_recovery_date() const { return recovery_date - offset; }
  UNIT_TEST_VIRTUAL int get_susceptible_date() const {
    if (recovery_period > -1) {
      return get_recovery_date() + recovery_period;
    } else {
      return INT_MAX;
    }
  }
  UNIT_TEST_VIRTUAL int set_susceptibility_period(int period) {
    return susceptibility_period = period;
  }
  UNIT_TEST_VIRTUAL int get_unsusceptible_date() const {
    return exposure_date + susceptibility_period;
  }

  UNIT_TEST_VIRTUAL void modify_asymptomatic_period(double multp, int cur_day);
  UNIT_TEST_VIRTUAL void modify_symptomatic_period(double multp, int cur_day);
  UNIT_TEST_VIRTUAL void modify_infectious_period(double multp, int cur_day);

  // parameters
  bool is_infectious() { return infectivity > trajectory_infectivity_threshold; }
  UNIT_TEST_VIRTUAL bool is_symptomatic() { return symptoms > trajectory_symptomaticity_threshold; }
  UNIT_TEST_VIRTUAL double get_susceptibility() const { return susceptibility; }

  UNIT_TEST_VIRTUAL double get_symptoms() const { return symptoms; }
  UNIT_TEST_VIRTUAL void modify_develops_symptoms(bool symptoms, int cur_day);
  UNIT_TEST_VIRTUAL void modify_susceptibility(double multp) { susceptibility *= multp; }
  UNIT_TEST_VIRTUAL void modify_infectivity(double multp) { infectivity_multp *= multp; }

  UNIT_TEST_VIRTUAL double get_infectivity(int day) const {
    day = day - exposure_date + offset;
    Trajectory::point point = trajectory->get_data_point(day);
    return point.infectivity * infectivity_multp;
  }

  void transmit(Person *infectee, Transmission *transmission);
  void addTransmission(Transmission *transmission);
  void setTrajectory(Trajectory *trajectory);

  // returns an infection for the given host and disease with exposed date and
  // recovered date both equal to day (instant resistance to the given disease);
  static Infection* get_dummy_infection(Disease *s, Person *host, int day);

  Trajectory * get_trajectory() { return trajectory; }

private:
  // associated disease
  Disease *disease;
  int id;

  // infection status (E/I/i/R)
  char status;
  bool isSusceptible;

  // chrono data
  int exposure_date;

  int latent_period;
  int asymptomatic_period;
  int symptomatic_period;
  int recovery_period;
  int susceptibility_period;

  // people involved
  Person *infector;
  Person *host;

  // where infection was caught
  Place *place;

  // number of people infected by this infection
  int infectee_count;

  // parameters
  bool will_be_symptomatic;
  double susceptibility;
  double infectivity;
  double infectivity_multp;
  double symptoms;
  int infection_model;

  // trajectory contains vectors that describe the (tentative) infection course
  Trajectory * trajectory;
  // thresholds used by determine_transition_dates()
  double trajectory_infectivity_threshold;
  double trajectory_symptomaticity_threshold;

  int incubation_period;
  // the transition dates are set in the constructor by determine_transition_dates()
  int infectious_date;
  int symptomatic_date;
  int asymptomatic_date;
  int recovery_date;
  int susceptible_date;

  void determine_transition_dates();

  // offset is used for dummy infections to advance to the first infectious day
  // this offset is applied in the get_*_date() methods defined in the header
  // as well as in the update() method when evaluating the trajectory
  bool dummy;
  int offset;

  std::vector <Transmission *> transmissions;

protected:
  // for mocks
  Infection() { }
};

#endif // _FRED_INFECTION_H


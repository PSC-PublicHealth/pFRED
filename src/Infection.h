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
class Health;
class Person;
class Place;
class Disease;
class Antiviral;
class Health;

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
  UNIT_TEST_VIRTUAL char get_disease_status() const { return status; }
  UNIT_TEST_VIRTUAL void become_infectious();
  UNIT_TEST_VIRTUAL void become_symptomatic();
  UNIT_TEST_VIRTUAL void become_susceptible();
  UNIT_TEST_VIRTUAL void recover();
  UNIT_TEST_VIRTUAL void remove();
  UNIT_TEST_VIRTUAL void update(int today);
  UNIT_TEST_VIRTUAL bool possibly_mutate(Health *health, int day); 	// may cause mutation and/or alter infection course
  bool is_infectious() const { return (status == 'I' || status == 'i'); }
  
  // general
  UNIT_TEST_VIRTUAL Disease *get_disease() const { return disease; }
  UNIT_TEST_VIRTUAL Person *get_infector() const { return infector; }
  UNIT_TEST_VIRTUAL Place *get_infected_place() const { return place; }
  UNIT_TEST_VIRTUAL int get_infectee_count() const { return infectee_count; }
  UNIT_TEST_VIRTUAL int add_infectee() { return ++infectee_count; }
  UNIT_TEST_VIRTUAL void print() const;
	
  // chrono
  UNIT_TEST_VIRTUAL int get_exposure_date() const { return exposure_date; }
  UNIT_TEST_VIRTUAL int get_infectious_date() const { return exposure_date + latent_period; }
  UNIT_TEST_VIRTUAL int get_symptomatic_date() const { return get_infectious_date() + asymptomatic_period; }
  UNIT_TEST_VIRTUAL int get_recovery_date() const { return get_symptomatic_date() + symptomatic_period; }
  UNIT_TEST_VIRTUAL int get_susceptible_date() const;
  UNIT_TEST_VIRTUAL void modify_asymptomatic_period(double multp, int cur_day);
  UNIT_TEST_VIRTUAL void modify_symptomatic_period(double multp, int cur_day);
  UNIT_TEST_VIRTUAL void modify_infectious_period(double multp, int cur_day);
	
  // parameters
  UNIT_TEST_VIRTUAL bool is_symptomatic() const { return symptoms > 0; }
  UNIT_TEST_VIRTUAL double get_susceptibility() const { return susceptibility; }
  UNIT_TEST_VIRTUAL double get_infectivity() const { return infectivity * infectivity_multp; }
  UNIT_TEST_VIRTUAL double get_symptoms() const { return symptoms; }
  UNIT_TEST_VIRTUAL void modify_develops_symptoms(bool symptoms, int cur_day);
  UNIT_TEST_VIRTUAL void modify_susceptibility(double multp) { susceptibility *= multp; }
  UNIT_TEST_VIRTUAL void modify_infectivity(double multp) { infectivity_multp = multp; }	
	
  // returns an infection for the given host and disease with exposed date and
  // recovered date both equal to day (instant resistance to the given disease);
  static Infection* get_dummy_infection(Disease *s, Person *host, int day);
  
 private:
  // associated disease
  Disease *disease;
  int id;
	
  // infection status (E/I/i/R)
  char status;
	
  // chrono data
  int exposure_date;
  int latent_period;
  int asymptomatic_period;
  int symptomatic_period;
  int recovery_period;
  
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
	
 protected:
  // for mocks
  Infection() { }
};

#endif // _FRED_INFECTION_H

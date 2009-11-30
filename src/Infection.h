/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Infection.h
//

#ifndef _FRED_INFECTION_H
#define _FRED_INFECTION_H

class Person;
class Place;
class Strain;

class Infection {
public:
  // Create an infection of type s, for person infectee, starting
  // on date day.
  // Place and infector may be null if this is a seed infection.
  // Place may be null if this is a infection due to a mutation event.
  Infection(Strain * s, Person* infector, Person* infectee,
	    Place* place, int day);
  void become_infectious();
  void recover();
  int is_symptomatic() { return (symptoms > 0); }
  Strain * get_strain() { return strain; }
  char get_strain_status() { return strain_status; }
  int get_exposure_date() { return exposure_date; }
  int get_infectious_date() { return infectious_date; }
  int get_recovered_date() { return recovered_date; }
  int get_infector();
  int get_infected_place_id();
  char get_infected_place_type();
  int get_infectees() { return infectees; }
  int add_infectee() { return ++infectees; }
  double get_susceptibility() { return susceptibility; }
  double get_infectivity() { return infectivity; }
  double get_symptoms() { return symptoms; }

  // May result in a mutation, which causes a new infection of a different
  // strain type in this host.  May also alter the course of this infection
  // (shortening or lengthening the duration).  
  bool possibly_mutate(int day);

private:
  // Change the future course of this infection.  Changing parameters such
  // that past transition dates are affected is invalid.  Invalid actions:
  //       - Calling this method after the infection's host is recovered
  //       - changing the number of latent days if the host of this infection
  //         is already infectious
  // Yes, you could bypass these checks by not passing the true current_day,
  // but that's just mean.
  void reset_infection_course(int num_latent_days, int num_infectious_days,
			      bool will_have_symptoms, int current_day);

  Strain * strain;
  char strain_status;
  int latent_period;
  int infectious_period;
  int exposure_date;
  int infectious_date;
  int recovered_date;
  Person* infector;
  Person* host;
  Place* infected_place;
  int infectees;
  int will_be_symptomatic;
  double susceptibility;
  double infectivity;
  double symptoms;
};

#endif // _FRED_INFECTION_H


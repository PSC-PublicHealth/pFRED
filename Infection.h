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

class Health;
class Person;
class Place;
class Strain;
class Antiviral;
class Health;

class Infection {
public:
  // Create an infection of type s, for person infectee, starting
  // on date day.
  // Place and infector may be null if this is a seed infection.
  // Place may be null if this is an infection due to a mutation event.
  Infection(Strain * s, Person* infector, Person* infectee,
            Place* place, int day);
  void become_infectious();
  void become_symptomatic();
  void recover();
  // Transition between latent, asymptomatic and symptomatic states, based on
  // the current day and this Infection's course.
  void update(int day);
  int is_symptomatic() { return (symptoms > 0); }
  Strain * get_strain() { return strain; }
  char get_strain_status() { return strain_status; }
  int get_exposure_date() { return exposure_date; }
  int get_infectious_date() { return infectious_date; }
  int get_symptomatic_date() {return symptomatic_date; }
  int get_recovered_date() { return recovered_date; }
  int get_susceptible_date() { return susceptible_date; }
  int get_infector();
  int get_infected_place_id();
  char get_infected_place_type();
  int get_infectees() { return infectees; }
  int add_infectee() { return ++infectees; }
  double get_susceptibility() { return susceptibility; }
  double get_infectivity() { return infectivity * infectivity_multp; }
  double get_symptoms() { return symptoms; }
  
  //Modifiers
  void modify_susceptibility(double multp){ susceptibility*=multp; }
  void modify_infectivity(double multp){ infectivity_multp = multp; }
  
  // Current day is needed to modify infectious/symptomatic periods,
  // because we can't cause this infection to recover in the past.
  void modify_asymptomatic_period(double multp, int cur_day);
  void modify_symptomatic_period(double multp, int cur_day);
  
  // Modifying infectious period is equivalent to modifying asymptomatic and
  // symptomatic period by the same factor (in that order).
  void modify_infectious_period(double multp, int cur_day);
  // Can only call this if the person hasn't already developed symptoms.
  void modify_develops_symptoms(bool symptoms, int cur_day);
  // May result in a mutation, which causes a new infection of a different
  // strain type in this host.  May also alter the course of this infection
  // (shortening or lengthening the duration).  
  bool possibly_mutate(Health* health, int day);
  
  void print();
  
  // Returns an infection for the given host and strain with exposed date and
  // recovered date both equal to day (instant resistance to the given strain);
  static Infection* get_dummy_infection(Strain *s, Person* host, int day);
  
private:
  // Change the future course of this infection.  Changing parameters such
  // that past transition dates are affected is invalid.  Invalid actions:
  //       - Calling this method after the host recovered
  //       - changing the number of latent days if the host of this infection
  //         is already infectious
  // Yes, you could bypass these checks by not passing the true current_day,
  // but that's just mean.
  void reset_infection_course(int num_latent_days, int num_asymp_days,
                              int num_symp_days, int current_day);
  
  Strain * strain;
  char strain_status;
  int latent_period;
  int infectious_period;
  int asymp_period;
  int symp_period;
  int exposure_date;
  int infectious_date;
  int symptomatic_date;
  int recovered_date;
  int susceptible_date;
  Person* infector;
  Person* host;
  Place* infected_place;
  int infectees;
  int will_be_symptomatic;
  double susceptibility;
  double infectivity;
  double infectivity_multp;
  double symptoms;
};

#endif // _FRED_INFECTION_H


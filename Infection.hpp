/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Infection.hpp
//

#ifndef _FRED_INFECTION_H
#define _FRED_INFECTION_H

class Infection {
public:
  Infection(int id, int d, int person_id, int loc, char place_type, int day);
  void become_infectious(int id, int strain);
  void recover(int id, int strain);
  int is_symptomatic();
  char get_strain_status() { return strain_status; }
  double get_susceptibility() { return susceptibility; }
  double get_infectivity() { return infectivity; }
  int get_exposure_date() { return exposure_date; }
  int get_infectious_date() { return infectious_date; }
  int get_recovered_date() { return recovered_date; }
  int get_infector() { return infector; }
  int get_infected_place() { return infected_place; }
  char get_infected_place_type() { return infected_place_type; }
  int get_infectees() { return infectees; }
  int add_infectee() { return ++infectees; }

private:
  char strain_status;
  int latent_period;
  int infectious_period;
  int exposure_date;
  int infectious_date;
  int recovered_date;
  int infector;
  int infected_place;
  char infected_place_type;
  int infectees;
  double susceptibility;
  double infectivity;
};

#endif // _FRED_INFECTION_H


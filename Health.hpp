/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.hpp
//

#ifndef _FRED_HEALTH_H
#define _FRED_HEALTH_H

class Health {
public:
  Health(int id);
  void make_susceptible(int id, int strain);
  void make_exposed(int id, int d, int person_id, int loc, char place_type, int day);
  void make_infectious(int id, int strain);
  void make_recovered(int id, int strain);
  int is_symptomatic();
  char get_strain_status(int d) { return strain_status[d]; }
  double get_susceptibility(int d) { return susceptibility[d]; }
  double get_infectivity(int d) { return infectivity[d]; }
  int get_exposure_date(int d) { return exposure_date[d]; }
  int get_infectious_date(int d) { return infectious_date[d]; }
  int get_recovered_date(int d) { return recovered_date[d]; }
  int get_infector(int d) { return infector[d]; }
  int get_infected_place(int d) { return infected_place[d]; }
  char get_infected_place_type(int d) { return infected_place_type[d]; }
  int get_infectees(int d) { return infectees[d]; }
  int add_infectee(int d) { return ++infectees[d]; }

private:
  char *strain_status;
  int *latent_period;
  int *infectious_period;
  int *exposure_date;
  int *infectious_date;
  int *recovered_date;
  int *infector;
  int *infected_place;
  char *infected_place_type;
  int *infectees;
  double *susceptibility;
  double *infectivity;
};

#endif // _FRED_HEALTH_H


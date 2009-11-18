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

#include <stdlib.h>
#include <vector>
using namespace std;

#define MAX_STRAINS 2

class Infection;

class Health {
public:
  Health();
  void reset();
  void become_exposed(int id, int strain, int person_id, int loc, char place_type, int day);
  void become_infectious(int id, int strain);
  void recover(int id, int strain);
  int is_symptomatic();
  char get_strain_status(int strain);
  int add_infectee(int strain);
  int get_exposure_date(int strain);
  int get_infectious_date(int strain);
  int get_recovered_date(int strain);
  int get_infector(int strain);
  int get_infected_place(int strain);
  char get_infected_place_type(int strain);
  int get_infectees(int strain);
  double get_susceptibility(int strain);
  double get_infectivity(int strain);

private:
  vector <Infection *> *infection;
};

#endif // _FRED_HEALTH_H


// -*- C++ -*-
/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Health.h
//

#ifndef _FRED_HEALTH_H
#define _FRED_HEALTH_H

#include <vector>
using namespace std;

#include "Infection.h"

class Person;
class Infection;
class Strain;
class Antiviral;
class Antivirals;

class Health {
public:
  Health(Person * person);
  void reset();
  void update(int day);
  void become_exposed(Infection *inf);
  void become_infectious(Strain * strain);
  void recover(Strain * strain);
  int is_symptomatic();
  inline char get_strain_status(int strain) {
    if (infection[strain].empty())
      return 'S';
    else 
      return infection[strain][0]->get_strain_status();
  }

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
  int is_on_av(int day,int strain); // Returns position in array of av if applicable
  //Medication operators
  int take(Antiviral *av, int day); //Return 1 if taken
  int get_number_av_taken(void){ return number_av_taken;}
  int get_antiviral_start_date(int i){ return antiviral_start_date[i];}
  //Modifiers
  void modify_susceptibility(int strain, double multp);
  void modify_infectivity(int strain, double multp);
  void modify_infectious_period(int strain, double multp);

  //int set_antiviral_date(int day); // returns 1 if can't be set.
  //void set_av(Antiviral *AV) { av = AV; }
   
private:
  Person * self;
  int strains;
  vector <Infection *> *infection;
  // help!!!
  int number_av_taken;
  Antivirals * AVs;
  vector < int > antiviral_start_date;
  vector < Antiviral * > avs;
};

#endif // _FRED_HEALTH_H


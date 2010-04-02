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
#include "Strain.h"

class Person;
class Infection;
class Strain;
class Antiviral;
class Antivirals;
class AV_Manager;
class AV_Health;
class Vaccine;
class Vaccine_Health;

class Health {
public:
  Health(Person * person);
  ~Health(void);
  void reset();
  void update(int day);
  void update_mutations(int day);
  void become_susceptible(int strain);
  void become_exposed(Infection *inf);
  void become_infectious(Strain * strain);
  void become_immune(Strain* strain);
  void recover(Strain * strain);
  int is_symptomatic();
  bool is_immune(Strain* strain){ return immunity[strain->get_id()];}
  inline char get_strain_status(int strain) {
    if(immunity[strain] == 1){
      return 'M';
    }
    else{
      if (infection[strain] == NULL)
	return 'S';
      else 
	return infection[strain]->get_strain_status();
    }
    
  }

  Person* get_self(){ return self;}
  int get_num_strains(void) { return strains; }
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
  Infection* get_infection(int strain) { return infection[strain]; }
  bool is_on_av_for_strain(int day, int strain);

  //Medication operators
  void take(Vaccine *vacc, int day);
  void take(Antiviral *av, int day);
  int get_number_av_taken(void)             const { return av_health.size();}
  int get_checked_for_av(int s)             const { return checked_for_av[s]; }
  void flip_checked_for_av(int s) { checked_for_av[s] = 1; } 
  bool is_vaccinated(void) { 
    if(vaccine_health.size() > 0){return true;} 
    else {return false;} 
  } // no strain yet!!!!
  int get_number_vaccines_taken(void)        const { return vaccine_health.size();}
  AV_Health* get_av_health(int i)            const { return av_health[i];}
  Vaccine_Health* get_vaccine_health(int i)  const { return vaccine_health[i];}

  //Modifiers
  void modify_susceptibility(int strain, double multp);
  void modify_infectivity(int strain, double multp);

  // Current day is needed to modify infectious period, because we can't cause this
  // infection to recover in the past.
  // Modifying infectious period is equivalent to modifying symptomatic and asymptomatic
  // periods by the same amount.
  void modify_infectious_period(int strain, double multp, int cur_day);
  void modify_symptomatic_period(int strain, double multp, int cur_day);
  void modify_asymptomatic_period(int strain, double multp, int cur_day);
  // Can't change develops_symptoms if this person is not asymptomatic ('i' or 'E')
  void modify_develops_symptoms(int strain, bool symptoms, int cur_day);
   
private:
  Person * self;
  int strains;
  Infection ** infection;
  vector < bool > immunity;
  double *susceptibility_multp;
  vector < bool > checked_for_av;
  vector < AV_Health * > av_health;
  vector < Vaccine_Health * > vaccine_health;
};

#endif // _FRED_HEALTH_H


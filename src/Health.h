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
  ~Health();
  void reset();
  void update(int day);
  void become_susceptible(int strain);
  void become_exposed(Infection *inf);
  void become_infectious(Strain * strain);
  void become_symptomatic(Strain *strain);
  void become_immune(Strain* strain);
  void declare_at_risk(Strain* strain);
  void recover(Strain * strain);
  int is_symptomatic() const;
  bool is_immune(Strain* strain) const { return immunity[strain->get_id()]; }
  bool is_at_risk(Strain* strain) const { return at_risk[strain->get_id()]; }
  char get_strain_status (int strain) const;
  Person* get_self() const { return self;}
  int get_num_strains() const { return strains; }
  int add_infectee(int strain);
  int get_exposure_date(int strain) const;
  int get_infectious_date(int strain) const;
  int get_recovered_date(int strain) const;
  int get_symptomatic_date(int strain) const;
  int get_infector(int strain) const;
  int get_infected_place(int strain) const;
  char get_infected_place_type(int strain) const;
  int get_infectees(int strain) const;	
  double get_susceptibility(int strain) const;
  double get_infectivity(int strain) const;
  Infection* get_infection(int strain) const { return infection[strain]; }
  bool is_on_av_for_strain(int day, int strain) const;
  
  //Medication operators
  void take(Vaccine *vacc, int day);
  void take(Antiviral *av, int day);
  int get_number_av_taken()             const { return av_health.size();}
  int get_checked_for_av(int s)             const { return checked_for_av[s]; }
  void flip_checked_for_av(int s) { checked_for_av[s] = 1; } 
  bool is_vaccinated() const { return vaccine_health.size(); }
  int get_number_vaccines_taken()        const { return vaccine_health.size();}
  AV_Health* get_av_health(int i)            const { return av_health[i];}
  Vaccine_Health* get_vaccine_health(int i)  const { return vaccine_health[i];}
  bool takes_av;
  
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
  void update_mutations(int day);

  Person * self;
  int strains;
  Infection ** infection;
  vector < bool > immunity;
  vector < bool > at_risk;	// Agent is/isn't at risk for severe complications
  double *susceptibility_multp;
  vector < bool > checked_for_av;
  vector < AV_Health * > av_health;
  vector < Vaccine_Health * > vaccine_health;

protected:
  Health() { }
};

inline char Health::get_strain_status(int strain) const {
  if (immunity[strain])
    return 'M';
  else if (!infection[strain])
    return 'S';
  else 
    return infection[strain]->get_strain_status();
}

#endif // _FRED_HEALTH_H

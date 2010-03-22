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
#include "VaccineStatus.h"
using namespace std;

#include "Infection.h"

class Person;
class Infection;
class Strain;
class Antiviral;
class Antivirals;
class AV_Manager;
class AV_Status;
class Vaccine;
class Vaccine_Status;

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
  void recover(Strain * strain);
  void immunize(Strain* strain);
  int is_symptomatic();
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
  int is_on_av(int day, int strain);

  //Medication operators
  int take(Antiviral *av, int day); //Return 1 if taken
  int take(Vaccine *vacc, int day); 
  int get_number_av_taken(void){ return av_stats.size();}
  int get_checked_for_av(int s) { return checked_for_av[s]; }
  void flip_checked_for_av(int s) { checked_for_av[s] = 1; }
  AV_Status* get_av_stat(int i) { return av_stats[i];} 
  int is_vaccinated(void) { if(vaccine_stats.size() > 0){return 1;} else {return 0;} } // no strain yet!!!!
  int get_number_vaccines_taken(void) { return vaccine_stats.size();}
  Vaccine_Status* get_vaccine_stat(int i){ return vaccine_stats[i];}

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
  void immunize(int strain);
   
private:
  Person * self;
  int strains;
  Infection ** infection;
  vector < int > immunity;
  double *susceptibility_multp;
  vector <int > checked_for_av;
  vector < AV_Status * > av_stats;
  vector < Vaccine_Status * > vaccine_stats;
};

#endif // _FRED_HEALTH_H


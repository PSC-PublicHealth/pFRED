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
#include "Disease.h"

class Person;
class Infection;
class Disease;
class Antiviral;
class Antivirals;
class AV_Manager;
class AV_Health;
class Vaccine;
class Vaccine_Health;
class Vaccine_Manager;

class Health {
public:

  /**
   * Default constructor that sets the Person to which this Health object applies
   *
   * @param person a pointer to a Person object
   */
  Health(Person * person);

  ~Health();

  /**
   * Perform the daily update for this object
   *
   * @param day the simulation day
   */
  void update(int day);

  /**
   * Agent is susceptible to the disease
   *
   * @param disease which disease
   */
  void become_susceptible(int disease);

  /**
   * Agent is unsusceptible to the disease
   *
   * @param disease which disease
   */
  void become_unsusceptible(int disease);

  /**
   * Agent is exposed to an Infection
   *
   * @param inf a pointer to the Infection object
   */
  void become_exposed(Infection *inf);

  /**
   * Agent is infectious
   *
   * @param disease pointer to the Disease object
   */
  void become_infectious(Disease * disease);

  /**
   * Agent is symptomatic
   *
   * @param disease pointer to the Disease object
   */
  void become_symptomatic(Disease *disease);

  /**
   * Agent is immune to the disease
   *
   * @param disease pointer to the Disease object
   */
  void become_immune(Disease* disease);

  /**
   * Agent is removed from the susceptible population to a given disease
   *
   * @param disease which disease
   */
  void become_removed(int disease);

  /**
   * Agent is 'At Risk' to a given Disease
   *
   * @param disease pointer to the Disease object
   */
  void declare_at_risk(Disease* disease);

  /**
   * Agent recovers from a given Disease
   *
   * @param disease pointer to the Disease object
   */
  void recover(Disease * disease);

  /**
   * Is the agent susceptible to a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is susceptible, <code>false</code> otherwise
   */
  bool is_susceptible (int disease) const {
    return susceptible[disease];
  }

  /**
   * Is the agent infectious for a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is infectious, <code>false</code> otherwise
   */
  bool is_infectious(int disease) const {
    return (status[disease] == 'I' || status[disease] == 'i');
  }

  /**
   * Is the agent symptomatic - note that this is independent of disease
   *
   * @return <code>true</code> if the agent is symptomatic, <code>false</code> otherwise
   */
  bool is_symptomatic() const;

  /**
   * Is the agent immune to a given disease
   *
   * @param disease pointer to the disease in question
   * @return <code>true</code> if the agent is immune, <code>false</code> otherwise
   */
  bool is_immune(Disease* disease) const {
    return immunity[disease->get_id()];
  }

  /**
   * Is the agent 'At Risk' to a given disease
   *
   * @param disease pointer to the disease in question
   * @return <code>true</code> if the agent is at risk, <code>false</code> otherwise
   */
  bool is_at_risk(Disease* disease) const {
    return at_risk[disease->get_id()];
  }

  /**
   * Is the agent 'At Risk' to a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is at risk, <code>false</code> otherwise
   */
  bool is_at_risk(int disease) const {
    return at_risk[disease];
  }

  /**
   * Get the agent's status for a given disease
   *
   * @param disease which disease
   * @return the agent's status
   */
  char get_disease_status (int disease) const {
    return status[disease];
  }

  /**
   * Get the Person object with which this Health object is associated
   *
   * @return a pointer to a Person object
   */
  Person* get_self() const {
    return self;
  }

  /**
   *@return the count of diseases
   */
  int get_num_diseases() const {
    return diseases;
  }

  /**
   * @param disease
   * @return
   */
  int add_infectee(int disease);

  /**
   * @param disease
   * @return the simulation day that this agent became exposed to the disease
   */
  int get_exposure_date(int disease) const;

  /**
   * @param disease
   * @return the simulation day that this agent became infectious with the disease
   */
  int get_infectious_date(int disease) const;

  /**
   * @param disease
   * @return the simulation day that this agent recovered from the disease
   */
  int get_recovered_date(int disease) const;

  /**
   * @param disease the disease to check
   * @return the simulation day that this agent became symptomatic to the disease
   */
  int get_symptomatic_date(int disease) const;

  /**
   * @param disease the disease to check
   * @return
   */
  int get_infector(int disease) const;

  /**
   * @param disease the disease to check
   * @return
   */
  int get_infected_place(int disease) const;

  /**
   * @param disease the disease to check
   * @return the label of the infected place
   */
  char * get_infected_place_label(int disease) const;

  /**
   * @param disease the disease to check
   * @return the type of place
   */
  char get_infected_place_type(int disease) const;

  /**
   * @param disease the disease to check
   * @return
   */
  int get_infectees(int disease) const;

  /**
   * @param disease the disease to check
   * @return
   */
  double get_susceptibility(int disease) const;

  /**
   * @param disease the disease to check
   * @param day the simulation day
   * @return
   */
  double get_infectivity(int disease, int day) const;

  /**
   * @param disease the disease to check
   * @return a pointer to the Infection object
   */
  Infection* get_infection(int disease) const {
    return infection[disease];
  }

  /**
   * Determine whether or not the agent is on Anti-Virals for a given disease on a given day
   *
   * @param day the simulation day
   * @param disease the disease to check
   * @return <code>true</code> if the agent is on Anti-Virals, <code>false</code> otherwise
   */
  bool is_on_av_for_disease(int day, int disease) const;

  /**
   * Infect an agent with a disease
   *
   * @param infectee a pointer to the Person that this agent is trying to infect
   * @param disease the disease with which to infect the Person
   * @param transmission a pointer to a Transmission object
   */
  void infect(Person *infectee, int disease, Transmission *transmission);

  /**
   * @param disease pointer to a Disease object
   * @param transmission pointer to a Transmission object
   */
  void getInfected(Disease *disease, Transmission *transmission);

  //Medication operators
  /**
   * Agent will take a vaccine
   * @param vacc pointer to the Vaccine to take
   * @param day the simulation day
   * @param vm a pointer to the Manager of the Vaccinations
   */
  void take(Vaccine *vacc, int day, Vaccine_Manager* vm);

  /**
   * Agent will take an antiviral
   * @param av pointer to the Antiviral to take
   * @param day the simulation day
   */
  void take(Antiviral *av, int day);

  /**
   * @return a count of the antivirals this agent has already taken
   */
  int get_number_av_taken()             const {
    return av_health.size();
  }

  /**
   * @param s the index of the av to check
   * @return the checked_for_av with the given index
   */
  int get_checked_for_av(int s)             const {
    return checked_for_av[s];
  }

  /**
   * Set the checked_for_av value at the given index to 1
   * @param s the index of the av to set
   */
  void flip_checked_for_av(int s) {
    checked_for_av[s] = 1;
  }

  /**
   * @return <code>true</code> if the agent is vaccinated, <code>false</code> if not
   */
  bool is_vaccinated() const {
    return vaccine_health.size();
  }

  /**
   * @return the number of vaccines this agent has taken
   */
  int get_number_vaccines_taken()        const {
    return vaccine_health.size();
  }

  /**
   * @return a pointer to this instance's AV_Health object
   */
  AV_Health * get_av_health(int i)            const {
    return av_health[i];
  }

  /**
   * @return a pointer to this instance's Vaccine_Health object
   */
  Vaccine_Health * get_vaccine_health(int i)  const {
    return vaccine_health[i];
  }
  bool takes_av;
  bool takes_vaccine;

  //Modifiers
  /**
   * Alter the susceptibility of the agent to the given disease by a multiplier
   * @param disease the disease to which the agent is suceptible
   * @param multp the multiplier to apply
   */
  void modify_susceptibility(int disease, double multp);

  /**
   * Alter the infectivity of the agent to the given disease by a multiplier
   * @param disease the disease with which the agent is infectious
   * @param multp the multiplier to apply
   */
  void modify_infectivity(int disease, double multp);

  /**
   * Alter the infectious period of the agent for the given disease by a multiplier.
   * Modifying infectious period is equivalent to modifying symptomatic and asymptomatic
   * periods by the same amount. Current day is needed to modify infectious period, because we can't cause this
   * infection to recover in the past.
   *
   * @param disease the disease with which the agent is infectious
   * @param multp the multiplier to apply
   * @param cur_day the simulation day
   */
  void modify_infectious_period(int disease, double multp, int cur_day);

  /**
   * Alter the symptomatic period of the agent for the given disease by a multiplier.
   * Current day is needed to modify symptomatic period, because we can't cause this
   * infection to recover in the past.
   *
   * @param disease the disease with which the agent is symptomatic
   * @param multp the multiplier to apply
   * @param cur_day the simulation day
   */
  void modify_symptomatic_period(int disease, double multp, int cur_day);

  /**
   * Alter the asymptomatic period of the agent for the given disease by a multiplier.
   * Current day is needed to modify symptomatic period, because we can't cause this
   * infection to recover in the past.
   *
   * @param disease the disease with which the agent is asymptomatic
   * @param multp the multiplier to apply
   * @param cur_day the simulation day
   */
  void modify_asymptomatic_period(int disease, double multp, int cur_day);

  /**
   * Alter the whether or not the agent will develop symptoms.
   * Can't change develops_symptoms if this person is not asymptomatic ('i' or 'E')
   * Current day is needed to modify symptomatic period, because we can't change symptomaticity that
   * is in the past.
   *
   * @param disease the disease with which the agent is asymptomatic
   * @param symptoms whether or not the agent is showing symptoms
   * @param cur_day the simulation day
   */
  void modify_develops_symptoms(int disease, bool symptoms, int cur_day);

private:
  Person * self;
  int diseases;
  Infection **infection;
  vector < bool > immunity;
  vector < bool > at_risk;  // Agent is/isn't at risk for severe complications
  double *susceptibility_multp;
  vector < bool > checked_for_av;
  vector < AV_Health * > av_health;
  vector < Vaccine_Health * > vaccine_health;
  char * status;
  bool * susceptible;
  bool symptomatic_status;
protected:
  Health() { }
};

#endif // _FRED_HEALTH_H

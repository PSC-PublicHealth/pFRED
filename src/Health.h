/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Health.h
//

#ifndef _FRED_HEALTH_H
#define _FRED_HEALTH_H

#include <vector>
#include <bitset>
using namespace std;

#include "Infection.h"
#include "Disease.h"
#include "Global.h"
#include "Past_Infection.h"

class Person;
class Disease;
class Antiviral;
class Antivirals;
class AV_Manager;
class AV_Health;
class Vaccine;
class Vaccine_Health;
class Vaccine_Manager;

class Health {

  static int nantivirals;

public:

  /**
   * Default constructor that sets the Person to which this Health object applies
   *
   * @param person a pointer to a Person object
   */
  Health(Person * person);

  ~Health();

  /**
   *
   * Perform the daily update for this object.  This method is performance-critical and
   * accounts for much of the time spent when the epidemic is otherwise 'idle' (little active
   * infection spreading).  Any changes made here should be made carefully,
   * especially w.r.t. cache performance and branch predictability.
   *
   * @param day the simulation day
   */
  void update( Person * self, int day );

  /*
   * Separating the updates for vaccine & antivirals from the
   * infection update gives improvement for the base.  
   */
  void update_interventions( Person * self, int day );

  /**
   * Agent is susceptible to the disease
   *
   * @param disease which disease
   */
  void become_susceptible( Person * self, int disease_id );

  void become_susceptible( Person * self, Disease * disease );

  /**
   * Agent is unsusceptible to the disease
   *
   * @param disease which disease
   */
  void become_unsusceptible( Person * self, Disease * disease );

  /**
   * Agent is infectious
   *
   * @param disease pointer to the Disease object
   */
  void become_infectious( Person * self, Disease * disease );

  /**
   * Agent is symptomatic
   *
   * @param disease pointer to the Disease object
   */
  void become_symptomatic( Person * self, Disease * disease );

  /**
   * Agent is immune to the disease
   *
   * @param disease pointer to the Disease object
   */
  void become_immune( Person * self, Disease* disease );

  /**
   * Agent is removed from the susceptible population that is
   * available to a given disease
   *
   * @param disease which disease
   */
  void become_removed( Person * self, int disease_id);

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
  void recover( Person * self, Disease * disease );

  /**
   * Is the agent susceptible to a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is susceptible, <code>false</code> otherwise
   */
  bool is_susceptible (int disease_id) const {return susceptible.test( disease_id );}

  /**
   * Is the agent infectious for a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is infectious, <code>false</code> otherwise
   */
  bool is_infectious(int disease_id) const { return (infectious.test( disease_id ) ); }

  /**
   * Is the agent symptomatic - note that this is independent of disease
   *
   * @return <code>true</code> if the agent is symptomatic, <code>false</code> otherwise
   */
  bool is_symptomatic() const { return symptomatic.any(); }
  bool is_symptomatic(int disease_id) { return symptomatic.test( disease_id ); }

  /**
   * Is the agent immune to a given disease
   *
   * @param disease pointer to the disease in question
   * @return <code>true</code> if the agent is immune, <code>false</code> otherwise
   */
  bool is_immune(Disease* disease) const {
    return immunity.test( disease->get_id() );
  }

  /**
   * Is the agent 'At Risk' to a given disease
   *
   * @param disease pointer to the disease in question
   * @return <code>true</code> if the agent is at risk, <code>false</code> otherwise
   */
  bool is_at_risk(Disease* disease) const {
    return at_risk.test( disease->get_id() );
  }

  /**
   * Is the agent 'At Risk' to a given disease
   *
   * @param disease which disease
   * @return <code>true</code> if the agent is at risk, <code>false</code> otherwise
   */
  bool is_at_risk(int disease_id) const {
    return at_risk.test( disease_id );
  }

  /**
   * @param disease
   * @return the simulation day that this agent became exposed to the disease
   */
  int get_exposure_date(int disease_id) const;

  /**
   * @param disease
   * @return the simulation day that this agent became infectious with the disease
   */
  int get_infectious_date(int disease_id) const;

  /**
   * @param disease
   * @return the simulation day that this agent recovered from the disease
   */
  int get_recovered_date(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return the simulation day that this agent became symptomatic to the disease
   */
  int get_symptomatic_date(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return Person who gave this person the disease
   */
  Person * get_infector(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return
   */
  int get_infected_place(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return the label of the infected place
   */
  char * get_infected_place_label(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return the type of place
   */
  char get_infected_place_type(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return
   */
  int get_infectees(int disease_id) const;

  /**
   * @param disease the disease to check
   * @return
   */
  double get_susceptibility(int disease_id) const;

  /**
   * @param disease the disease to check
   * @param day the simulation day
   * @return
   */
  double get_infectivity(int disease_id, int day) const;

  /**
   * @param disease the disease to check
   * @return a pointer to the Infection object
   */
  Infection* get_infection(int disease_id) const {
    return infection[disease_id];
  }

  /**
   * Determine whether or not the agent is on Anti-Virals for a given disease on a given day
   *
   * @param day the simulation day
   * @param disease the disease to check
   * @return <code>true</code> if the agent is on Anti-Virals, <code>false</code> otherwise
   */
  bool is_on_av_for_disease(int day, int disease_id) const;

  /**
   * Infect an agent with a disease
   *
   * @param infectee a pointer to the Person that this agent is trying to infect
   * @param disease the disease with which to infect the Person
   * @param transmission a pointer to a Transmission object
   */
  void infect( Person * self, Person *infectee, int disease_id, Transmission & transmission);

  /**
   * @param disease pointer to a Disease object
   * @param transmission pointer to a Transmission object
   */
  void become_exposed( Person * self, Disease *disease, Transmission & transmission );

  //Medication operators
  /**
   * Agent will take a vaccine
   * @param vacc pointer to the Vaccine to take
   * @param day the simulation day
   * @param vm a pointer to the Manager of the Vaccinations
   */
  void take_vaccine( Person * self, Vaccine *vacc, int day, Vaccine_Manager* vm );

  /**
   * Agent will take an antiviral
   * @param av pointer to the Antiviral to take
   * @param day the simulation day
   */
  void take(Antiviral *av, int day);

  /**
   * @return a count of the antivirals this agent has already taken
   */
  int get_number_av_taken() const {
    if ( av_health ) {
      return av_health->size();
    }
    else {
      return 0;
    }
  }

  /**
   * @param s the index of the av to check
   * @return the checked_for_av with the given index
   */
  int get_checked_for_av(int s) const {
    assert( checked_for_av != NULL );
    return (*checked_for_av)[ s ];
  }

  /**
   * Set the checked_for_av value at the given index to 1
   * @param s the index of the av to set
   */
  void flip_checked_for_av(int s) {
    if ( checked_for_av == NULL ) {
      checked_for_av = new checked_for_av_type();
      checked_for_av->assign( nantivirals, false ); 
    }
    (*checked_for_av)[ s ] = 1;
  }

  /**
   * @return <code>true</code> if the agent is vaccinated, <code>false</code> if not
   */
  bool is_vaccinated() const {
    if ( vaccine_health ) {
      return vaccine_health->size();
    }
    else {
      return 0;
    }
  }

  /**
   * @return the number of vaccines this agent has taken
   */
  int get_number_vaccines_taken() const {
    if ( vaccine_health ) {
      return vaccine_health->size();
    }
    else {
      return 0;
    }
  }

  /**
   * @return a pointer to this instance's AV_Health object
   */
  AV_Health * get_av_health(int i) const {
    assert( av_health != NULL );
    return (*av_health)[ i ];
  }

  /**
   * @return this instance's av_start day
   */
  int get_av_start_day(int i) const;

  /**
   * @return a pointer to this instance's Vaccine_Health object
   */
  Vaccine_Health * get_vaccine_health(int i) const {
    if ( vaccine_health ) {
      return (*vaccine_health)[ i ];
    }
    else {
      return NULL;
    }
  }

  //Modifiers
  /**
   * Alter the susceptibility of the agent to the given disease by a multiplier
   * @param disease the disease to which the agent is suceptible
   * @param multp the multiplier to apply
   */
  void modify_susceptibility(int disease_id, double multp);

  /**
   * Alter the infectivity of the agent to the given disease by a multiplier
   * @param disease the disease with which the agent is infectious
   * @param multp the multiplier to apply
   */
  void modify_infectivity(int disease_id, double multp);

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
  void modify_infectious_period(int disease_id, double multp, int cur_day);

  /**
   * Alter the symptomatic period of the agent for the given disease by a multiplier.
   * Current day is needed to modify symptomatic period, because we can't cause this
   * infection to recover in the past.
   *
   * @param disease the disease with which the agent is symptomatic
   * @param multp the multiplier to apply
   * @param cur_day the simulation day
   */
  void modify_symptomatic_period(int disease_id, double multp, int cur_day);

  /**
   * Alter the asymptomatic period of the agent for the given disease by a multiplier.
   * Current day is needed to modify symptomatic period, because we can't cause this
   * infection to recover in the past.
   *
   * @param disease the disease with which the agent is asymptomatic
   * @param multp the multiplier to apply
   * @param cur_day the simulation day
   */
  void modify_asymptomatic_period(int disease_id, double multp, int cur_day);

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
  void modify_develops_symptoms(int disease_id, bool symptoms, int cur_day);

  void terminate( Person * self );

  int get_num_past_infections( int disease ) { return past_infections[ disease ].size(); }
  
  Past_Infection * get_past_infection( int disease, int i ) {
    return &( past_infections[ disease ].at( i ) );
  }
  
  void clear_past_infections( int disease ) { past_infections[ disease ].clear(); }

  //void add_past_infection(int d, Past_Infection *pi){ past_infections[d].push_back(pi); }  
  void add_past_infection( int strain_id, int recovery_date, int age_at_exposure, Disease * dis ) {
    past_infections[ dis->get_id() ].push_back( Past_Infection( strain_id, recovery_date, age_at_exposure ) );
  }

  void die() { alive = false; }

private:
  
  // The index of the person in the Population
  //int person_index;

  // The alive bool could probably be eliminated
  bool alive;
  
  // TODO (JVD): The infection vector & bitset should be combined into a little 
  // helper class to make sure that they're always synched up.
  // There would be just a little overhead in doing this but probably well worth it.
  // Until then make sure that they're always changed together.
  Infection * infection[ Global::MAX_NUM_DISEASES ];
  // bitset removes need to check each infection in above array to
  // find out if any are not NULL
  fred::disease_bitset active_infections;
  fred::disease_bitset immunity;
  fred::disease_bitset at_risk;  // Agent is/isn't at risk for severe complications
  // Per-disease health status flags
  fred::disease_bitset susceptible;
  fred::disease_bitset infectious;
  fred::disease_bitset symptomatic;
  fred::disease_bitset recovered_today;
  fred::disease_bitset evaluate_susceptibility;

  // per-disease susceptibility multiplier
  double susceptibility_multp[ Global::MAX_NUM_DISEASES ];
 
  // Antivirals.  These are all dynamically allocated to save space
  // when not in use
  typedef vector < bool > checked_for_av_type; 
  checked_for_av_type * checked_for_av;
  typedef vector < AV_Health * > av_health_type;
  av_health_type * av_health;
  typedef vector < AV_Health * >::iterator av_health_itr; 

  // Vaccines.  These are all dynamically allocated to save space
  // when not in use
  typedef vector < Vaccine_Health * > vaccine_health_type;
  vaccine_health_type * vaccine_health;
  typedef vector < Vaccine_Health * >::iterator vaccine_health_itr;

  // Define a bitset type to hold health flags
  // Enumeration corresponding to positions in health
  // intervention_flags bitset is defined in implementation file
  typedef std::bitset< 2 > intervention_flags_type;
  intervention_flags_type intervention_flags;

  // Past_Infections used for reignition
  vector < Past_Infection > past_infections[ Global::MAX_NUM_DISEASES ];
  
  int infectee_count[ Global::MAX_NUM_DISEASES ];
  int susceptible_date[ Global::MAX_NUM_DISEASES ];

protected:
  
  friend class Person;
  Health() { }
};

#endif // _FRED_HEALTH_H

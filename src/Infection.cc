/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

// Infection.cc
// ------------
// Stores the infectivity and symptomaticity trajectories that determine the transition dates for this person.

#include <stdexcept>
#include "limits.h"
#include <map>
#include <vector>
#include <float.h>
#include <string>
#include <sstream>

#include "Infection.h"
#include "Evolution.h"
#include "Transmission.h"
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Workplace.h"
#include "Random.h"
#include "Disease.h"
#include "Health.h"
#include "IntraHost.h"
#include "Activities.h"
#include "Utils.h"

using std::out_of_range;


Infection::Infection(Disease *disease, Person* infector, Person* host, Place* place, int day) {
 
  // flag for health updates
  Global::Pop.set_mask_by_index( fred::Update_Health, host->get_pop_index() );

  // general
  this->disease = disease;
  this->infector = infector;
  this->host = host;
  this->place = place;
  this->trajectory = NULL;
  infectee_count = 0;
  age_at_exposure = host->get_age();

  is_susceptible = true;
  trajectory_infectivity_threshold = disease->get_infectivity_threshold();
  trajectory_symptomaticity_threshold = disease->get_symptomaticity_threshold();

  // parameters
  infectivity_multp = 1.0;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;

  // chrono
  latent_period = 0;
  asymptomatic_period = 0;
  symptomatic_period = 0;
  incubation_period = 0;

  infectious_date = -1;
  symptomatic_date = -1;
  asymptomatic_date = -1;
  recovery_date = -1;

  will_be_symptomatic = false;

  exposure_date = day;
  recovery_period = disease->get_days_recovered();

  // Determine if this infection produces an immune response
  immune_response = disease->gen_immunity_infection( host->get_age() );
}

Infection::~Infection() {
  delete trajectory;
}

void Infection::determine_transition_dates() {
  // returns the first date that the agent changes state
  bool was_latent = true;
  bool was_incubating = true;

  latent_period = 0;
  asymptomatic_period = 0;
  symptomatic_period = 0;
  incubation_period = 0;

  infectious_date = -1;
  symptomatic_date = -1;
  asymptomatic_date = -1;
  recovery_date = exposure_date + trajectory->get_duration();

  Trajectory::iterator trj_it = Trajectory::iterator(trajectory);

  while (trj_it.has_next()) {
    bool infective = (trj_it.next().infectivity > trajectory_infectivity_threshold);
    bool symptomatic = (trj_it.next().symptomaticity > trajectory_symptomaticity_threshold);
    bool asymptomatic = (infective && !(symptomatic));

    if (infective && was_latent) {
      latent_period = trj_it.get_current();
      infectious_date = exposure_date + latent_period; // TODO

      if (asymptomatic & was_latent) {
        asymptomatic_date = infectious_date;
      }

      was_latent = false;
    }

    if (symptomatic && was_incubating) {
      incubation_period = trj_it.get_current();
      symptomatic_date = exposure_date + incubation_period;
      was_incubating = false;
    }

    if (symptomatic) {
      symptomatic_period++;
    }

    if (asymptomatic) {
      asymptomatic_period++;
    }
  }
  // report_infection(day);
}

void Infection::update(int today) {
  if(trajectory == NULL) return;

  int days_post_exposure = today - exposure_date;

  Trajectory::point trajectory_point = trajectory->get_data_point(days_post_exposure);
  infectivity = trajectory_point.infectivity;
  symptoms = trajectory_point.symptomaticity;

  if (today == get_infectious_date()) {
    host->become_infectious(disease);
  }

  if (today == get_symptomatic_date()) {
    host->become_symptomatic(disease);
  }

  if (today == get_recovery_date()) {
    host->recover(disease);
  }
 
  if (today == get_unsusceptible_date()) {
    host->become_unsusceptible(disease);
    is_susceptible = false;
  }
  
  if(today != get_recovery_date()) {
    vector<int> strains;
    trajectory->get_all_strains(strains);
  }
}

void Infection::advance_seed_infection( int days_to_advance ) {
  // only valid for seed infections!
  assert( recovery_date != -1 );
  assert( exposure_date != -1 );
  exposure_date -= days_to_advance;
  determine_transition_dates();
  if ( get_infectious_date() <= 0 + Global::Epidemic_offset ) {
    host->become_infectious( disease );
  }
  if ( get_symptomatic_date() <= 0 + Global::Epidemic_offset ) {
    host->become_symptomatic( disease );
  }
  if ( get_recovery_date() <= 0 + Global::Epidemic_offset ) {
    host->recover( disease );
  }
  if ( get_unsusceptible_date() <= 0 + Global::Epidemic_offset ) {
    host->become_unsusceptible( disease );
  }
}

void Infection::modify_symptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");

  // after symptomatic period
  if (today >= get_recovery_date())
    throw out_of_range("cannot modify: past symptomatic period");

  // before symptomatic period
  else if (today < get_symptomatic_date()) {
    trajectory->modify_symp_period(symptomatic_date, symptomatic_period * multp);
    determine_transition_dates();
  }

  // during symptomatic period
  // if days_left becomes 0, we make it 1 so that update() sees the new dates
  else {
    //int days_into = today - get_symptomatic_date();
    int days_left = get_recovery_date() - today;
    days_left *= multp;
    trajectory->modify_symp_period(today - exposure_date, days_left);
    determine_transition_dates();
  }
}

void Infection::modify_asymptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");

  // after asymptomatic period
  if (today >= get_symptomatic_date()) {
    printf("ERROR: Person %d %d %d\n", host->get_id(), today, get_symptomatic_date());
    throw out_of_range("cannot modify: past asymptomatic period");
  }

  // before asymptomatic period
  else if (today < get_infectious_date()) {
    trajectory->modify_asymp_period(exposure_date, asymptomatic_period * multp, get_symptomatic_date());
    determine_transition_dates();
  }

  // during asymptomatic period
  else {
    //int days_into = today - get_infectious_date();
    int days_left = get_symptomatic_date() - today;
    trajectory->modify_asymp_period(today - exposure_date, days_left * multp, get_symptomatic_date());
    determine_transition_dates();
  }
}

void Infection::modify_infectious_period(double multp, int today) {
  if (today < get_symptomatic_date())
    modify_asymptomatic_period(multp, today);

  modify_symptomatic_period(multp, today);
}

void Infection::modify_develops_symptoms(bool symptoms, int today) {
  if ((today >= get_symptomatic_date() && get_asymptomatic_date() == -1) || (today >= get_recovery_date()) )
    throw out_of_range("cannot modify: past symptomatic period");

  if (will_be_symptomatic != symptoms) {
    symptomatic_period = will_be_symptomatic ? disease->get_days_symp() : 0;
    trajectory->modify_develops_symp(get_symptomatic_date(), symptomatic_period);
    determine_transition_dates();
  }
}

void Infection::print() const {
  printf("INF: Infection of disease type: %i in person %i "
         "periods:  latent %i, asymp: %i, symp: %i recovery: %i "
         "dates: exposed: %i, infectious: %i, symptomatic: %i, recovered: %i susceptible: %i "
         "will have symp? %i, suscept: %.3f infectivity: %.3f "
         "infectivity_multp: %.3f symptms: %.3f\n",
         disease->get_id(),
         host->get_id(),
         latent_period,
         asymptomatic_period,
         symptomatic_period,
         recovery_period,
         exposure_date,
         get_infectious_date(),
         get_symptomatic_date(),
         get_recovery_date(),
         get_susceptible_date(),
         will_be_symptomatic,
         susceptibility,
         infectivity,
         infectivity_multp,
         symptoms);
}

void Infection::transmit(Person *infectee, Transmission & transmission) {
  int day = transmission.get_exposure_date() - exposure_date;
  Transmission::Loads * loads = trajectory->getInoculum( day );
  transmission.set_initial_loads( loads );
  infectee->become_exposed( this->disease, transmission );
}

void Infection::setTrajectory( Trajectory * _trajectory ) {
  trajectory = _trajectory;
  determine_transition_dates();
}

void Infection::report_infection(int day) const {
  if (Global::Infectionfp == NULL) return;

  int place_id = place == NULL? -1 : place->get_id();
  char place_type = place == NULL? 'X' : place->get_type();
  int place_size = place == NULL? -1: place->get_size();
  if (place_type == 'O' || place_type == 'C') {
    Place *container = place->get_container();
    place_size = container->get_size();
  }

  std::stringstream infStrS;
  infStrS.precision(3);
  infStrS << fixed << "day "<< day << " dis " << disease->get_id() << " host " << host->get_id()
	  << " age " << host->get_real_age() << " sick_leave " << host->is_sick_leave_available()
	  << " infector " << (infector == NULL ? -1 : infector->get_id())
	  << " inf_age " << (infector == NULL ? -1 : infector->get_real_age())
	  << " inf_sympt " << (infector == NULL ? -1 : infector->is_symptomatic())
	  << " inf_sick_leave " << (infector == NULL ? -1 : infector->is_sick_leave_available())
	  << " at " << place_type << " place " << place_id
	  << " size " << place_size << " is_teacher " << (int) host->is_teacher();

  if (Global::Track_infection_events > 1)
    infStrS << "| PERIODS  latent " << latent_period << " asymp " << asymptomatic_period
	    << " symp " << symptomatic_period << " recovery " << recovery_period;

  if (Global::Track_infection_events > 2)
    infStrS << "| DATES exp " << exposure_date << " inf " << get_infectious_date()
	    << " symp " << get_symptomatic_date() << " rec " << get_recovery_date()
	    << " sus " << get_susceptible_date();

  if (Global::Track_infection_events > 3)
    infStrS << "| will_by_symp? " << will_be_symptomatic 
	    << " sucs " << susceptibility 
	    << " infect " << infectivity 
	    << " inf_multp " << infectivity_multp 
	    << " sympts " << symptoms;

  infStrS << "\n";

  fprintf(Global::Infectionfp, "%s", infStrS.str().c_str());
  // flush performed at the end of every day so that it doesn't gum up multithreading
  //fflush(Global::Infectionfp);
}

int Infection::get_num_past_infections()
{ 
  return host->get_num_past_infections( disease->get_id() ); 
}
  
Past_Infection *Infection::get_past_infection(int i)
{ 
  return host->get_past_infection( disease->get_id(), i); 
}



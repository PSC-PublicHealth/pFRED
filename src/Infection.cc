/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

// Infection.cc
// ------------
// Determines the transition dates for this person.

#include <stdexcept>
#include "limits.h"

#include "Infection.h"
#include "Evolution.h"
#include "Transmission.h"
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Health.h"
#include "IHM.h"
#include <map>
#include <vector>
#include <float.h>

using std::out_of_range;

Infection::Infection(Disease *disease, Person* infector, Person* host, Place* place, int day) {
	
	// general
  this->disease = disease;
  this->id = disease->get_id();
  this->infector = infector;
  this->host = host;
  this->place = place;
  this->trajectory = NULL;
  infectee_count = 0;
	
//	trajectory_infectivity_threshold = IHM::singletonIHM().get_infectivity_threshold(this);
//	trajectory_symptomaticity_threshold = IHM::singletonIHM().get_symptomaticity_threshold(this);

  isSusceptible = true;
  trajectory_infectivity_threshold = 0;
  trajectory_symptomaticity_threshold = 0;

	// parameters
  infectivity_multp = 1.0;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  
	// chrono
  exposure_date = day;
  recovery_period = disease->get_days_recovered();

	offset = 0;
	
	if (symptomatic_date != -1) { will_be_symptomatic = true; }
	else { will_be_symptomatic = false; }

	host->set_changed();
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
}

void Infection::become_infectious() {
  disease->decrement_E_count();
	if (is_symptomatic()) {
    status = 'I';
    disease->increment_I_count();
    disease->increment_c_count();
	}
	else {
 		status = 'i';
    disease->increment_i_count();
  } 
  host->set_changed();
}

void Infection::become_susceptible() {
  status = 'S';
  disease->decrement_r_count();
}

void Infection::become_unsusceptible() {
  isSusceptible = false;
  disease->decrement_S_count();
	host->set_changed();
}

void Infection::become_symptomatic() {
	if (status == 'i') {
		disease->decrement_i_count();
	}
	if (status != 'I') {
		status = 'I';
		disease->increment_I_count();
	}
	host->set_changed();
}

void Infection::recover() {
 	if (status == 'I') { 
		disease->decrement_I_count();
	 }
	else if (status == 'i') {
    disease->decrement_i_count();
	}
	
	status = 'R';

  if (recovery_period > -1) { 
    disease->increment_r_count();
	}
  else {
    disease->increment_R_count();
	}
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  host->set_changed(); // note that the infection state changed
}

void Infection::update(int today) {
  if(trajectory == NULL) return;

	// offset used for dummy infections only
	int days_post_exposure = today - exposure_date + offset;

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
	if (today == get_susceptible_date()) {
    host->become_susceptible(disease->get_id());
	}
  if (today == get_unsusceptible_date()) {
//    host->become_unsusceptible(disease->get_id());
  }
}

void Infection::modify_symptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");
	
  // after symptomatic period
  if (today >= get_recovery_date())
    throw out_of_range("cannot modify: past symptomatic period");

//	IHM::singletonIHM().modify_symptomatic_period(this, multp, today);
	determine_transition_dates();

  host->set_changed();
}

void Infection::modify_asymptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");
	
  // after asymptomatic period
  if (today >= get_symptomatic_date()){
    printf("ERROR: Person %d %d %d\n", host->get_id(), today, get_symptomatic_date());
    throw out_of_range("cannot modify: past asymptomatic period");
  }

  // TODO
//	trajectory = IHM::singletonIHM().modify_asymptomatic_period(this, multp, today);
	determine_transition_dates();

  host->set_changed();
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
//		trajectory = IHM::singletonIHM().modify_develops_symptoms(this, symptoms, today);
		determine_transition_dates();
  	host->set_changed();
	}
}
// TODO re-write to use trajectories, or remove
bool Infection::possibly_mutate(Health* health, int day) {
	// this will not work with the new trajectory system
	return false;
  // too late
  if (day >= get_recovery_date())
    return false;
	
  // attempt mutation
  Disease* new_disease = disease->should_mutate_to();
  if (!new_disease)
    return false;
	
  if (Verbose)
    fprintf(Statusfp, "Person %i will mutate from disease %i to disease %i on day %i\n",
            host->get_id(), disease->get_id(), new_disease->get_id(), day);
	
  // mutated infection
  Infection *mutated = new Infection(new_disease, infector, host, NULL, day);
	
  // we're past the latent period
  if (day >= get_infectious_date()) 
    mutated->latent_period = 0;
  
  // we're past the asympomatic period, we must be symptomatic
  if (day >= get_symptomatic_date()) {
    mutated->asymptomatic_period = 0;
    mutated->modify_develops_symptoms(true, day);
  }
	
  // switch to mutated infection
  exposure_date = day;
	
  // update current infection's course
  switch (status) {
  case 'I':
    symptomatic_period = day - get_symptomatic_date();
    break;
  case 'i':
    symptomatic_period = 0;
    asymptomatic_period = day - get_infectious_date();
    break;
  case 'E':
    symptomatic_period = 0;
    asymptomatic_period = 0;
    latent_period = day - get_exposure_date();
    break;
  default:
    throw out_of_range("should be either of E/i/I while mutating");
  } 
	
  // recover from current infection
  host->recover(disease);
	
  // expose to mutated infection
  host->become_exposed(mutated);
	
  // note that the infection state changed
  host->set_changed();
	
  return true;
}

void Infection::print() const {
  printf("Infection of disease type: %i in person %i current status: %c\n"
         "periods:  latent %i, asymp: %i, symp: %i recovery: %i \n"
         "dates: exposed: %i, infectious: %i, symptomatic: %i, recovered: %i susceptible: %i\n"
         "will have symp? %i, suscept: %.3f infectivity: %.3f "
         "infectivity_multp: %.3f symptms: %.3f\n",
         disease->get_id(),
         host->get_id(),
         status,
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

// static 
Infection *Infection::get_dummy_infection(Disease *s, Person* host, int day) {
  Infection* i = new Infection(s, NULL, host, NULL, day);
	i->dummy = true;
	i->offset = i->infectious_date - day;
  return i;
}

void Infection::transmit(Person *infectee, Transmission *transmission) {
  int day = transmission->get_exposure_date() - exposure_date + offset;
  map<int, double> *loads = trajectory->getInoculum(day);
  transmission->setInitialLoads(loads);
  infectee->getInfected(this->disease, transmission);
}

void Infection::addTransmission(Transmission *transmission) {
  // Consider transmission only if it is within 24 hours
  if(transmission->get_exposure_date() != this->get_exposure_date()){
    if(Verbose > 1){
      printf("New transmission failed for person %d because it is after 24 hours.\n", host->get_id());
    }
    return;
  }
  
  transmissions.push_back(transmission);

  int day = transmission->get_exposure_date() - exposure_date + offset;
  
  map<int, double> *loads = transmission->getInitialLoads();

  if(trajectory != NULL){ // this person is already infected by current disease
    map<int, double> *currentLoads = trajectory->getCurrentLoads(day);

    // Ignore further transmissions of the same strain
    for( map<int, double>::iterator it = currentLoads->begin(); it != currentLoads->end(); it++ ){
      loads->insert( pair<int, double> (it->first, it->second) );
    }
  }

  disease->get_evolution()->doEvolution(this, loads);

  // TODO update in case infectious date etc is today
  // update(exposure_date);

  if(infectious_date == -1){
    trajectory = NULL;

    if(Verbose > 1){
      printf("New transmission failed for person %d. \n", host->get_id());
    }
  }
  else{
    if(Verbose > 1){
      printf("Transmission succeeded for person %d.\n", host->get_id());
      printf("New trajectories:\n");
      trajectory->print();
      printf("Dates: %d, %d, %d, %d, %d \n", exposure_date, infectious_date,	symptomatic_date,	asymptomatic_date, recovery_date);
   }

    status = 'E';
    disease->increment_E_count();
    disease->increment_C_count();
    disease->insert_into_infected_list(host);
    
    if(isSusceptible && susceptibility_period == 0){
      host->become_unsusceptible(disease->get_id());
    }
  }  
}

void Infection::setTrajectory(Trajectory *trajectory) {
  this->trajectory = trajectory;
  this->determine_transition_dates();
}


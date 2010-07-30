/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

// Infection.cc
// ------------
// Determines the transition dates for this person.

#include <stdexcept>

#include "Infection.h"
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Strain.h"
#include "Health.h"

using std::out_of_range;

Infection::Infection(Strain *strain, Person* infector, Person* host, Place* place, int day) {
	// general
	this->strain = strain;
	this->infector = infector;
	this->host = host;
	this->place = place;
	infectee_count = 0;
	
	// status
	status = 'E';
	
	// parameters
	will_be_symptomatic = strain->get_symptoms();
	infectivity_multp = 1.0;
	infectivity = 0.0;
	susceptibility = 0.0;
	symptoms = 0.0;
	
	// chrono
	exposure_date = day;
	asymptomatic_period = symptomatic_period = 0;
	latent_period = strain->get_days_latent();
	recovery_period = strain->get_days_recovered();
	
	if (SEiIR_model) { // SEiIR model
		asymptomatic_period = strain->get_days_asymp();
		if (will_be_symptomatic)
			symptomatic_period = strain->get_days_symp();
	} else { // SEIR/SEiR model
		if (will_be_symptomatic)
			symptomatic_period = strain->get_days_symp();
		else
			asymptomatic_period = strain->get_days_asymp();
	}
  
	host->set_changed();
}

void Infection::become_infectious() {
	if (SEiIR_model) { // SEiIR model
		status = 'i';
		infectivity = strain->get_asymp_infectivity();
		symptoms = 0.0;
	} else { // SEIR/SEiR model
		if (will_be_symptomatic) {
			status = 'I';
			infectivity = strain->get_symp_infectivity();
			symptoms = 1.0;
		} else {
			status = 'i';
			infectivity = strain->get_asymp_infectivity();
			symptoms = 0.0;
		}
	}
	
	host->set_changed();
}

void Infection::become_susceptible() {
	status = 'S';
}

void Infection::become_symptomatic() {
  status = 'I';
  infectivity = strain->get_symp_infectivity();
  symptoms = 1.0;
  
  host->set_changed();
}

void Infection::recover() {
	status = 'R';
	infectivity = 0.0;
	susceptibility = 0.0;
	symptoms = 0.0;
	
	host->set_changed(); // note that the infection state changed
}

void Infection::update(int today) {
	char status = get_strain_status();
	
	if (status == 'E' && today == get_infectious_date()) {
		host->become_infectious(strain);
		status = get_strain_status();
	}
	
	if (SEiIR_model) {  // SEiIR model
		if (status == 'i' && today == get_symptomatic_date()) {
			host->become_symptomatic(strain);
			status = get_strain_status();
		}
		if (status == 'I' && today == get_recovery_date()) {
			host->recover(strain);
			status = get_strain_status();
		}
	} else {  // SEIR/SEiR model
		if ((status == 'I' || status == 'i') && today == get_recovery_date()) {
			host->recover(strain);
			status = get_strain_status();
		}
	}
	
	if (status == 'R' && today == get_susceptible_date())
		become_susceptible();
}

void Infection::modify_symptomatic_period(double multp, int today) {
	// negative multiplier
	if (multp < 0)
		throw out_of_range("cannot modify: negative multiplier");
	
	// after symptomatic period
	if (today >= get_recovery_date())
		throw out_of_range("cannot modify: past symptomatic period");
	
	// before symptomatic period
	else if (today < get_symptomatic_date())
		symptomatic_period *= multp;
	
	// during symptomatic period
	// if days_left becomes 0, we make it 1 so that update() sees the new dates
	else {
		int days_into = today - get_symptomatic_date();
		int days_left = get_recovery_date() - today;
		days_left *= multp;
		symptomatic_period = days_into + (days_left ? days_left : 1);
	}
	
	host->set_changed();
}

void Infection::modify_asymptomatic_period(double multp, int today) {
	// negative multiplier
	if (multp < 0)
		throw out_of_range("cannot modify: negative multiplier");
	
	// after asymptomatic period
	if (today >= get_symptomatic_date())
		throw out_of_range("cannot modify: past asymptomatic period");
	
	// before asymptomatic period
	else if (today < get_infectious_date())
		asymptomatic_period *= multp;
	
	// during asymptomatic period
	// if days_left becomes 0, we make it 1 so that update() sees the new dates
	else {
		int days_into = today - get_infectious_date();
		int days_left = get_symptomatic_date() - today;
		days_left *= multp;
		asymptomatic_period = days_into + (days_left ? days_left : 1);
	}
	
	host->set_changed();
}

void Infection::modify_infectious_period(double multp, int today) {
	if (today < get_symptomatic_date())
		modify_asymptomatic_period(multp, today);
  modify_symptomatic_period(multp, today);
}

void Infection::modify_develops_symptoms(bool symptoms, int today) {
	if (today >= get_symptomatic_date())
	  throw out_of_range("cannot modify: past symptomatic period");
	
	will_be_symptomatic = symptoms;
	symptomatic_period = will_be_symptomatic ? strain->get_days_symp() : 0;
  
	host->set_changed();
}

bool Infection::possibly_mutate(Health* health, int day) {
	// too late
	if (day >= get_recovery_date())
		return false;
	
	// attempt mutation
	Strain* new_strain = strain->should_mutate_to();
	if (!new_strain)
		return false;
	
	if (Verbose)
		fprintf(Statusfp, "Person %i will mutate from strain %i to strain %i on day %i\n",
            host->get_id(), strain->get_id(), new_strain->get_id(), day);
	
	// mutated infection
	Infection *mutated = new Infection(new_strain, infector, host, NULL, day);
	
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
	switch (get_strain_status()) {
		case 'I':
			symptomatic_period = day - get_symptomatic_date();
			break;
		case 'i':
			symptomatic_period = 0;
			asymptomatic_period = day - get_infectious_date();
		case 'E':
			symptomatic_period = 0;
			asymptomatic_period = 0;
			latent_period = day - get_exposure_date();
		default:
			throw out_of_range("should be either of E/i/I while mutating");
	} 
	
	// recover from current infection
	host->recover(strain);
	
	// expose to mutated infection
	host->become_exposed(mutated);
	
	// note that the infection state changed
	host->set_changed();
	
	return true;
}

void Infection::print() const {
	printf("Infection of strain type: %i in person %i current status: %c\n"
         "periods:  latent %i, asymp: %i, symp: %i \n"
         "dates: exposed: %i, infectious: %i, symptomatic: %i, recovered: %i susceptible: %i\n"
         "will have symp? %i, suscept: %.3f infectivity: %.3f"
         "infectivity_multp: %.3f symptms: %.3f\n",
         strain->get_id(),
         host->get_id(),
         status,
         latent_period,
         asymptomatic_period,
         symptomatic_period,
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
Infection *Infection::get_dummy_infection(Strain *s, Person* host, int day) {
	Infection* i = new Infection(s, NULL, host, NULL, day);
	i->latent_period = 0;
	i->asymptomatic_period = 0;
	i->symptomatic_period = 0;
	return i;
}

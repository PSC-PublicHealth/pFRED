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
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Health.h"

using std::out_of_range;

Infection::Infection(Disease *disease, Person* infector, Person* host, Place* place, int day) {
  // general
  this->disease = disease;
  this->id = disease->get_id();
  this->infector = infector;
  this->host = host;
  this->place = place;
  infectee_count = 0;
	
  // status
  status = 'E';
  disease->decrement_S_count();
  disease->increment_E_count();
  disease->increment_C_count();
  disease->insert_into_infected_list(host);
	
  // parameters
  will_be_symptomatic = disease->get_symptoms();
  infectivity_multp = 1.0;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  infection_model = disease->get_infection_model();
	
  // chrono
  exposure_date = day;
  asymptomatic_period = symptomatic_period = 0;
  latent_period = disease->get_days_latent();
  recovery_period = disease->get_days_recovered();
	
  if (infection_model == SEQUENTIAL) { // SEiIR model
    asymptomatic_period = disease->get_days_asymp();
    if (will_be_symptomatic)
      symptomatic_period = disease->get_days_symp();
  } else { // SEIR/SEiR model
    if (will_be_symptomatic)
      symptomatic_period = disease->get_days_symp();
    else
      asymptomatic_period = disease->get_days_asymp();
  }
  report_event();
  host->set_changed();
}

void Infection::become_infectious() {
  disease->decrement_E_count();
  if (infection_model == SEQUENTIAL) { // SEiIR model
    status = 'i';
    disease->increment_i_count();
    infectivity = disease->get_asymp_infectivity();
    symptoms = 0.0;
  } else { // SEIR/SEiR model
    if (will_be_symptomatic) {
      status = 'I';
      disease->increment_I_count();
      infectivity = disease->get_symp_infectivity();
      symptoms = 1.0;
      disease->increment_c_count();
    } else {
      status = 'i';
      disease->increment_i_count();
      infectivity = disease->get_asymp_infectivity();
      symptoms = 0.0;
    }
  }
  host->set_changed();
}

void Infection::become_susceptible() {
  status = 'S';
  disease->decrement_r_count();
}

void Infection::become_symptomatic() {
  status = 'I';
  disease->decrement_i_count();
  disease->increment_I_count();
  infectivity = disease->get_symp_infectivity();
  symptoms = 1.0;
  host->set_changed();
}

void Infection::recover() {
  status = 'R';
  if (is_symptomatic())
    disease->decrement_I_count();
  else
    disease->decrement_i_count();
  if (recovery_period > -1) 
    disease->increment_r_count();
  else
    disease->increment_R_count();
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  host->set_changed(); // note that the infection state changed
}

void Infection::remove() {
  if (status == 'R') {
    return;
  }
  if (status == 'E') {
    disease->decrement_E_count();
  }
  if (status == 'I') {
    disease->decrement_I_count();
  }
  if (status == 'i') {
    disease->decrement_i_count();
  }
  status = 'R';
  host->set_changed(); // note that the infection state changed
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
}

void Infection::update(int today) {
  if (status == 'E' && today == get_infectious_date()) {
    host->become_infectious(disease);
  }
	
  if (infection_model == SEQUENTIAL) {  // SEiIR model
    if (status == 'i' && today == get_symptomatic_date()) {
      host->become_symptomatic(disease);
    }
    if (status == 'I' && today == get_recovery_date()) {
      host->recover(disease);
    }
  } else {  // SEIR/SEiR model
    if ((status == 'I' || status == 'i') && today == get_recovery_date()) {
      host->recover(disease);
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
  symptomatic_period = will_be_symptomatic ? disease->get_days_symp() : 0;
  
  host->set_changed();
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
  i->latent_period = 0;
  i->asymptomatic_period = 0;
  i->symptomatic_period = 0;
  return i;
}

int Infection::get_susceptible_date() const {
  if (recovery_period > -1) {
    return get_recovery_date() + recovery_period;
  } else {
    return INT_MAX;
  }
}


void Infection::report_event() const {
  if (Eventfp == NULL) return;
  fprintf(Eventfp, "INFECTION dis %d host %d "
	  " from %d at %s "
	  "| PERIODS  latent %d, asymp %d, symp %d recovery %d "
	  "| DATES exp %d, inf %d, symp %d, rec %d sus %d "
	  "| will_be_symp? %d, susc %.3f infect %.3f "
	  "inf_multp %.3f symptms %.3f\n",
	  id,
	  host->get_id(),
	  infector == NULL ? -1 : infector->get_id(),
	  place == NULL ? "X" : place->get_label(),
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
  fflush(Eventfp);
}


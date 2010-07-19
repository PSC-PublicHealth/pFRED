/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Infection.cc
//

#include "Infection.h"

// For logging purposes.
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Strain.h"
#include "Health.h"

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

Infection::Infection (Strain * s, Person* person_infector,
                      Person* infectee, Place* place, int day) {
  strain = s;
  strain_status = 'E';
  exposure_date = day;
  infectivity_multp = 1.0;
  will_be_symptomatic = strain->get_symptoms();
  latent_period = strain->get_days_latent();
  
  if (SEiIR_model) {
    // Determine health transition dates using SEiIR model
    asymp_period = strain->get_days_asymp();// * asymp_period_multp;
    if (will_be_symptomatic)
      symp_period = strain->get_days_symp();// * asymp_period_multp;
    else 
      symp_period = 0;
  }
  else {
    // Determine health transition dates using (SEIR or SEiR) model
    if (will_be_symptomatic) {
      symp_period = strain->get_days_symp(); // * symp_period_multp;
      asymp_period = 0;
    }
    else {
      symp_period = 0; 
      asymp_period = strain->get_days_asymp(); // * asymp_period_multp;
    }
  }
  
  infectious_date = exposure_date + latent_period;
  symptomatic_date = infectious_date + asymp_period;
  recovered_date = symptomatic_date + symp_period;
  int days_rec = strain->get_days_recovered();
  if (days_rec != -1) {
    susceptible_date = recovered_date + days_rec;
  } else {
    susceptible_date = -1;
  }
  infector = person_infector;
  infected_place = place;
  host = infectee;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  infectees = 0;
  host->set_changed(); // note that the infection state changed
}

// This logic shouldn't be here - we should have a dummy infected place
// for seed infections.  We could call it the seed farm.  That would be
// funny.
int Infection::get_infected_place_id() {
  if (infected_place != NULL) {
    return infected_place->get_id();
  }
  return -1;
}

char Infection::get_infected_place_type() {
  if (infected_place != NULL) {
    return infected_place->get_type();
  }
  return 'X';
}

int Infection::get_infector() {
  if (infector != NULL) {
    return infector->get_id();
  }
  return -1;
}

void Infection::become_infectious() {
  if (SEiIR_model) {
    strain_status = 'i';
    infectivity = strain->get_asymp_infectivity();
    symptoms = 0.0;
  }
  else {
    if (will_be_symptomatic) {
      strain_status = 'I';
      infectivity = strain->get_symp_infectivity();
      symptoms = 1.0;
    }
    else {
      strain_status = 'i';
      infectivity = strain->get_asymp_infectivity();
      symptoms = 0.0;
    }
  }
  host->set_changed(); // note that the infection state changed
}

void Infection::become_symptomatic() {
  strain_status = 'I';
  infectivity = strain->get_symp_infectivity();
  symptoms = 1.0;
  host->set_changed(); // note that the infection state changed
}

void Infection::update(int day) {
  char status = get_strain_status();
  if (status == 'E' && day == get_infectious_date()) {
    host->become_infectious(strain);
    status = get_strain_status();
  }
  if (SEiIR_model) {
    if (status == 'i' && day == get_symptomatic_date()) {
      become_symptomatic();
      status = get_strain_status();
    }
    if (status == 'I' && day == get_recovered_date()) {
      host->recover(strain);
      status = get_strain_status();
    }
  }
  else {
    if ((status == 'I' || status == 'i') && day == get_recovered_date()) {
      host->recover(strain);
      status = get_strain_status();
    }
  }
  if (status == 'R' && day == get_susceptible_date()) {
    host->become_susceptible(strain->get_id());
  }
}


void Infection::recover() {
  strain_status = 'R';
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  host->set_changed(); // note that the infection state changed
}

void Infection::reset_infection_course(int num_latent_days, int num_asymp_days,
                                       int num_symp_days, int current_day) {
  // If my host has already recovered, changing any parameters is invalid.
  if (current_day > recovered_date) {
    printf("Infection from strain %i: attempt to change infection course after"
           " person %i is already recovered.  Recovery date was %i, and current "
           "date is %i\n",
           strain->get_id(), host->get_id(), recovered_date, current_day);
    fflush(stdout);
    abort();
  }
  // If my host is already infectious, changing the infectious date is invalid.
  if (current_day > infectious_date && latent_period != num_latent_days) {
    printf("Infection from strain %i: attempt to change latent period from % i "
           "to %i after person %i is already infectious.  Current infectious "
           "date is %i, new day would be %i\n",
           strain->get_id(), latent_period, num_latent_days, host->get_id(),
           infectious_date, exposure_date + num_latent_days);
    fflush(stdout);
    abort();
  }
  
  // If my host is already symptomatic, changing the symptomatic state is invalid.
  if (current_day > symptomatic_date &&
      current_day > num_symp_days + symptomatic_date) {
    printf("Infection from strain %i in person %i on day %i: attempt to "
           "change symptomatic period to end in the past. "
           "current_symptomatic date: %i, current symptomatic period: %i\n"
           "new symptomatic date: %i new symptomatic_period: %i  ",
           strain->get_id(), host->get_id(), current_day,
           symptomatic_date, symp_period,
           num_latent_days + num_asymp_days, num_symp_days);
    fflush(stdout);
    abort();
  }
  
  latent_period = num_latent_days;
  asymp_period = num_asymp_days;
  symp_period = num_symp_days;
  infectious_date = exposure_date + latent_period;
  symptomatic_date = infectious_date + asymp_period;
  will_be_symptomatic = (symp_period > 0);
  recovered_date = symptomatic_date + symp_period;
  
  // If my host is already infectious, changing the infectious date is invalid.
  if (current_day > recovered_date) {
    printf("Infection from strain %i in person %i on day %i: attempt to "
           "change recovery date to be in the past\n"
           "Passed num_latent_days %i, num_asymp_days %i, num_symp_days %i",
           strain->get_id(), host->get_id(), current_day,
           num_latent_days, num_asymp_days, num_symp_days);
    fflush(stdout);
    abort();
  }
  
  if (Verbose > 2) {
    fprintf(Statusfp, "reset strain - new infectious_date: %i new recovered date: %i\n",
            infectious_date, recovered_date);
    fflush(Statusfp);
  }
  host->set_changed(); // note that the infection state changed
}

bool Infection::possibly_mutate(Health* health, int day) {
  if (day > recovered_date) {
    printf("Cannot mutate a recovered infection "
           "strain %i person %i on date %i status is %c recovered date is %i\n",
           strain->get_id(), host->get_id(), day, strain_status, recovered_date);
    print();
    fflush(stdout);
    abort();
  }
  Strain* new_strain = strain->should_mutate_to();
  if (new_strain == NULL) {
    return false;
  }
  else {
    if (Verbose > 0) {
      fprintf(Statusfp,"Person %i will mutate from strain %i to strain %i on day %i\n",
              host->get_id(), strain->get_id(), new_strain->get_id(), day);
      fflush(Statusfp);
    }
    Infection* new_infection = new Infection(new_strain, host, host, NULL, day);
    int new_asymp_days =
    new_infection->get_symptomatic_date() - new_infection->get_infectious_date();
    int new_symp_days =
    new_infection->get_recovered_date() - new_infection->get_symptomatic_date();
    
    // If I'm already symptomatic, I can't mutate to an infection that 
    // doesn't have symptoms.
    if (symptoms > 0 && new_symp_days == 0) {
      new_infection->modify_develops_symptoms(true, day);
      new_symp_days =
      new_infection->get_recovered_date() - new_infection->get_symptomatic_date();
    }
    
    if (day >= infectious_date && day < symptomatic_date) {
      new_infection->reset_infection_course(0, new_asymp_days,
                                            new_symp_days, day);
      symptomatic_date = day;
      symp_period = 0;
    } else if (day >= symptomatic_date) {
      
      new_infection->reset_infection_course(0, 0,
                                            new_symp_days, day);
      symp_period = day - symptomatic_date;
    }
    
    recovered_date = day;
    host->become_exposed(new_infection);
    host->set_changed(); // note that the infection state changed
    return true;
  }
}

// static 
Infection* Infection::get_dummy_infection(Strain *s, Person* host, int day) {
  Infection* i = new Infection(s, NULL, host, NULL, day);
  i->reset_infection_course(0, 0, false, day);
  return i;
}

void Infection::modify_symptomatic_period(double multp, int cur_day){ 
  if (symp_period == 0) return;
  int residual_symp_period = symp_period;
  if (cur_day > symptomatic_date) {
    residual_symp_period = cur_day - recovered_date;
  }
  residual_symp_period *= multp;
  if (residual_symp_period < 1)
    residual_symp_period = 1;
  recovered_date = symptomatic_date + residual_symp_period;
  if (recovered_date <= cur_day) {
    recovered_date = cur_day + 1;
  }
  symp_period = recovered_date - symptomatic_date;
  host->set_changed(); // note that the infection state changed
}

void Infection::modify_asymptomatic_period(double multp, int cur_day){ 
  if (strain_status == 'I' || asymp_period == 0 || cur_day == symptomatic_date) {
    return;
  }
  int residual_asymp_period = symptomatic_date - cur_day;
  residual_asymp_period *= multp;
  if (residual_asymp_period < 1)
    residual_asymp_period = 1;
  symptomatic_date = cur_day + residual_asymp_period;
  asymp_period = symptomatic_date - infectious_date;
  recovered_date = symptomatic_date + symp_period;
  host->set_changed(); // note that the infection state changed
}

void Infection::modify_infectious_period(double multp, int cur_day) {
  if (cur_day < symptomatic_date)
    modify_asymptomatic_period(multp, cur_day);
  if (cur_day < recovered_date)
    modify_symptomatic_period(multp, cur_day);
}

void Infection::modify_develops_symptoms(bool symptoms, int cur_day) {
  if (cur_day < symptomatic_date) {
    if (symptoms != will_be_symptomatic) {
      will_be_symptomatic = symptoms;
      if (!will_be_symptomatic) {
        symp_period = 0;
        recovered_date = symptomatic_date;
      } 
      else {
        // Relagating the modification of the symptomatic period to the other function.
        symp_period = strain->get_days_symp();
        recovered_date = symptomatic_date + symp_period;
        host->set_changed(); // note that the infection state changed
      }
    }
  }
  else {
    if (Verbose > 1) {
      printf("Infection from strain %i: attempt to change symptomaticity "
             " person %i is already symptomatic.  Symptomatic "
             "date is %i, status is %c\n",
             strain->get_id(), host->get_id(),
             symptomatic_date, strain_status);
    }
  }
}

void Infection::print() {
  printf("Infection of strain type: %i in person %i current status: %c\n"
         "periods:  latent %i, asymp: %i, symp: %i \n"
         "dates: exposed: %i, infectious: %i, symptomatic: %i, recovered: %i susceptible: %i\n"
         "will have symp? %i, suscept: %.3f infectivity: %.3f"
         "infectivity_multp: %.3f symptms: %.3f\n",
         strain->get_id(),
         host->get_id(),
         strain_status,
         latent_period,
         asymp_period,
         symp_period,
         exposure_date,
         infectious_date,
         symptomatic_date,
         recovered_date,
         susceptible_date,
         will_be_symptomatic,
         susceptibility,
         infectivity,
         infectivity_multp,
         symptoms);
}

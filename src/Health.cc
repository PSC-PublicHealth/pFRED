/*
   Copyright 2009 by the University of Pittsburgh
   Licensed under the Academic Free License version 3.0
   See the file "LICENSE" for more information
   */

//
//
// File: Health.cc
//

#include <new>
#include <stdexcept>

#include "Health.h"
#include "Place.h"
#include "Person.h"
#include "Disease.h"
#include "Evolution.h"
#include "Infection.h"
#include "Antiviral.h"
#include "Population.h"
#include "Random.h"
#include "Global.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "AV_Health.h"
#include "Vaccine.h"
#include "Vaccine_Dose.h"
#include "Vaccine_Health.h"
#include "Vaccine_Manager.h"
#include "Transmission.h"

int nantivirals = -1; // This global variable needs to be removed
char dummy_label[8];
bool recovered_today = false;

Health::Health (Person * person) {
  self = person;
  infection = new Infection* [Global::Diseases];
  susceptibility_multp = new double [Global::Diseases];
  susceptible = new bool [Global::Diseases];
  infectious = new bool [Global::Diseases];
  symptomatic = new bool [Global::Diseases];
  infectee_count = new int [Global::Diseases];
  susceptible_date = new int [Global::Diseases];
  has_symptoms = false;
  alive = true;

  for (int disease_id = 0; disease_id < Global::Diseases; disease_id++) {
    infection[disease_id] = NULL;
    susceptible[disease_id] = false;
    infectious[disease_id] = false;
    symptomatic[disease_id] = false;
    infectee_count[disease_id] = 0;
    susceptible_date[disease_id] = -1;
    become_susceptible(disease_id);
  }

  if(nantivirals == -1) {
    Params::get_param_from_string("number_antivirals",&nantivirals);
  }

  immunity.assign(Global::Diseases,false);

  vaccine_health.clear();
  takes_vaccine = false;

  av_health.clear();
  takes_av = false;
  checked_for_av.assign(nantivirals,false);

  // Determine if the agent is at risk
  at_risk.assign(Global::Diseases,false);
  for(int disease_id = 0; disease_id < Global::Diseases; disease_id++) {
    Disease* disease = Global::Pop.get_disease(disease_id);

    if(!disease->get_at_risk()->is_empty()) {
      double at_risk_prob = disease->get_at_risk()->find_value(self->get_age());

      if(RANDOM() < at_risk_prob) { // Now a probability <=1.0
        declare_at_risk(disease);
      }
    }
  }
}

Health::~Health() {
  delete[] infection;
  delete[] susceptibility_multp;
  delete[] susceptible;
  delete[] infectious;
  delete[] symptomatic;
  delete[] infectee_count;

  for(unsigned int i=0; i<vaccine_health.size(); i++)
    delete vaccine_health[i];
  vaccine_health.clear();
  takes_vaccine = false;

  for(unsigned int i=0; i<av_health.size(); i++)
    delete av_health[i];
  av_health.clear();
  takes_av = false;
}

void Health::become_susceptible(int disease_id) {
  if (susceptible[disease_id])
      return;
  assert(infection[disease_id] == NULL);
  susceptibility_multp[disease_id] = 1.0;
  susceptible[disease_id] = true;
  Disease * disease = Global::Pop.get_disease(disease_id);
  disease->become_susceptible(self);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now SUSCEPTIBLE for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}

void Health::become_susceptible(Disease * disease) {
  become_susceptible(disease->get_id());
}

void Health::become_exposed(Disease *disease, Transmission *transmission) {
  if (Global::Verbose > 1) {
    if (!transmission->getInfectedPlace())
      fprintf(Global::Statusfp, "EXPOSED_DUMMY person %d to disease %d\n",
	      self->get_id(), disease->get_id());
    else
      fprintf(Global::Statusfp, "EXPOSED person %d to disease %d\n",
	      self->get_id(), disease->get_id());
  }
  int disease_id = disease->get_id();
  infectious[disease_id] = false;
  symptomatic[disease_id] = false;

  if(infection[disease_id] == NULL) {
    infection[disease_id] = new Infection(disease, transmission->getInfector(), this->get_self(),
					 transmission->getInfectedPlace(), transmission->get_exposure_date());
  }

  Infection *infection_ptr = infection[disease_id];
  infection_ptr->addTransmission(transmission);
  disease->become_exposed(self);
}

void Health::become_unsusceptible(Disease * disease) {
  int disease_id = disease->get_id();
  if (susceptible[disease_id] == false)
      return;
  susceptible[disease_id] = false;
  disease->become_unsusceptible(self);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now UNSUSCEPTIBLE for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}

void Health::become_infectious(Disease * disease) {
  assert(infection[disease_id] != NULL);
  int disease_id = disease->get_id();
  infectious[disease_id] = true;
  disease->become_infectious(self);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now INFECTIOUS for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}

void Health::become_symptomatic(Disease *disease) {
  assert(infection[disease_id] != NULL);
  int disease_id = disease->get_id();
  if (symptomatic[disease_id])
    return;
  symptomatic[disease_id] = true;
  has_symptoms = true;
  disease->become_symptomatic(self);
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now SYMPTOMATIC for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}


void Health::recover(Disease * disease) {
  assert(infection[disease_id] != NULL);
  int disease_id = disease->get_id();
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now RECOVERED for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
  become_removed(disease_id);
  recovered_today = true;
}

void Health::become_removed(int disease_id) {
  Disease * disease = Global::Pop.get_disease(disease_id);
  disease->become_removed(self,susceptible[disease_id],
			  infectious[disease_id],
			  symptomatic[disease_id]);
  susceptible[disease_id] = false;
  infectious[disease_id] = false;
  symptomatic[disease_id] = false;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now REMOVED for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}

void Health::become_immune(Disease *disease) {
  int disease_id = disease->get_id();
  disease->become_immune(self,susceptible[disease_id],
			  infectious[disease_id],
			  symptomatic[disease_id]);
  susceptible[disease_id] = false;
  infectious[disease_id] = false;
  symptomatic[disease_id] = false;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d is now IMMUNE for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }
}

void Health::update(int day) {
  if (alive == false) return;

  // if deceased, clear health status
  if (self->is_deceased()) {
    for (int disease_id = 0; disease_id < Global::Diseases; disease_id++) {
      become_removed(disease_id);
    }
    alive = false;
    return;
  }

  // update vaccine status
  if (takes_vaccine) {
    int size = (int) vaccine_health.size();
    for (int i = 0; i < size; i++)
      vaccine_health[i]->update(day, self->get_age());
  }

  // check if agent has symptons due to any disease
  has_symptoms = false;

  // update each infection
  for (int disease_id = 0; disease_id < Global::Diseases; disease_id++) {

    // set a flag to detect call to recover()
    recovered_today = false;

    // update the infection (if it exists)
    if (infection[disease_id] != NULL)
      infection[disease_id]->update(day);

    // if the infection_update called recover(), it is now safe to
    // collect the susceptible date and delete the Infection object
    if (recovered_today) {
      susceptible_date[disease_id] = infection[disease_id]->get_susceptible_date();
      delete infection[disease_id];
      infection[disease_id] = NULL;
    }

    // check for susceptibility due to loss of immunity
    if (day == susceptible_date[disease_id])
      become_susceptible(disease_id);

    // check if agent has symptons due to any disease
    if (symptomatic[disease_id]) has_symptoms = true;
  }

  // update antiviral status
  if (takes_av) {
    int size = av_health.size();
    for(int i = 0; i < size; i++)
      av_health[i]->update(day);
  }
}

void Health::declare_at_risk(Disease* disease) {
  int disease_id = disease->get_id();
  at_risk[disease_id] = true;
}

int Health::get_exposure_date(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else
    return infection[disease_id]->get_exposure_date();
}

int Health::get_infectious_date(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else
    return infection[disease_id]->get_infectious_date();
}

int Health::get_recovered_date(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else
    return infection[disease_id]->get_recovery_date();
}

int Health:: get_symptomatic_date(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else
    return infection[disease_id]->get_symptomatic_date();
}

int Health::get_infector(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else if (infection[disease_id]->get_infector() == NULL)
    return -1;
  else
    return infection[disease_id]->get_infector()->get_id();
}

int Health::get_infected_place(int disease_id) const {
  if (infection[disease_id] == NULL)
    return -1;
  else if (infection[disease_id]->get_infected_place() == NULL)
    return -1;
  else
    return infection[disease_id]->get_infected_place()->get_id();
}

char Health::get_infected_place_type(int disease_id) const {
  if (infection[disease_id] == NULL)
    return 'X';
  else if (infection[disease_id]->get_infected_place() == NULL)
    return 'X';
  else
    return infection[disease_id]->get_infected_place()->get_type();
}

char * Health::get_infected_place_label(int disease_id) const {
  if (infection[disease_id] == NULL) {
    strcpy(dummy_label, "-");
    return dummy_label;
  } else if (infection[disease_id]->get_infected_place() == NULL) {
    strcpy(dummy_label, "X");
    return dummy_label;
  } else
    return infection[disease_id]->get_infected_place()->get_label();
}

int Health::get_infectees(int disease_id) const {
  return infectee_count[disease_id];
}

double Health::get_susceptibility(int disease_id) const {
  double suscep_multp = susceptibility_multp[disease_id];

  if (infection[disease_id] == NULL)
    return suscep_multp;
  else
    return infection[disease_id]->get_susceptibility() * suscep_multp;
}

double Health::get_infectivity(int disease_id, int day) const {
  if (infection[disease_id] == NULL)
    return 0.0;
  else
    return infection[disease_id]->get_infectivity(day);
}

//Modify Operators
void Health::modify_susceptibility(int disease_id, double multp) {
  if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " susceptibility for disease "
                               << disease_id << " by " << multp << "\n";

  susceptibility_multp[disease_id] *= multp;
}

void Health::modify_infectivity(int disease_id, double multp) {
  if (infection[disease_id] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease_id
                                 << " by " << multp << "\n";

    infection[disease_id]->modify_infectivity(multp);
  }
}

void Health::modify_infectious_period(int disease_id, double multp, int cur_day) {
  if (infection[disease_id] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease_id
                                 << " by " << multp << "\n";

    infection[disease_id]->modify_infectious_period(multp, cur_day);
  }
}

void Health::modify_asymptomatic_period(int disease_id, double multp, int cur_day) {
  if (infection[disease_id] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " asymptomatic period  for disease " << disease_id
                                 << " by " << multp << "\n";

    infection[disease_id]->modify_asymptomatic_period(multp, cur_day);
  }
}

void Health::modify_symptomatic_period(int disease_id, double multp, int cur_day) {
  if (infection[disease_id] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomatic period  for disease " << disease_id
                                 << " by " << multp << "\n";

    infection[disease_id]->modify_symptomatic_period(multp, cur_day);
  }
}

void Health::modify_develops_symptoms(int disease_id, bool symptoms, int cur_day) {
  if (infection[disease_id] != NULL &&
      (infection[disease_id]->is_infectious() && !infection[disease_id]->is_symptomatic()) ||
      (!infection[disease_id]->is_infectious())) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomaticity  for disease " << disease_id
                                 << " to " << symptoms << "\n";

    infection[disease_id]->modify_develops_symptoms(symptoms, cur_day);
    symptomatic[disease_id] = true;
  }
}

//Medication operators
void Health::take(Vaccine* vaccine, int day, Vaccine_Manager* vm) {
  // Compliance will be somewhere else
  int age = self->get_age();
  // Is this our first dose?
  Vaccine_Health* vaccine_health_for_dose = NULL;

  for(unsigned int ivh = 0; ivh < vaccine_health.size(); ivh++) {
    if(vaccine_health[ivh]->get_vaccine() == vaccine) {
      vaccine_health_for_dose = vaccine_health[ivh];
    }
  }

  if(vaccine_health_for_dose == NULL) { // This is our first dose of this vaccine
    vaccine_health.push_back(new Vaccine_Health(day,vaccine,age,this,vm));
    takes_vaccine = true;
  } else { // Already have a dose, need to take the next dose
    vaccine_health_for_dose->update_for_next_dose(day,age);
  }

  if (Global::VaccineTracefp != NULL) {
    fprintf(Global::VaccineTracefp," id %7d vaccid %3d",
	    self->get_id(),vaccine_health[vaccine_health.size()-1]->get_vaccine()->get_ID());
    vaccine_health[vaccine_health.size()-1]->printTrace();
    fprintf(Global::VaccineTracefp,"\n");
  }

  return;
}

void Health::take(Antiviral* av, int day) {
  av_health.push_back(new AV_Health(day,av,this));
  takes_av = true;
  return;
}

bool Health::is_on_av_for_disease(int day, int d) const {
  for (unsigned int iav = 0; iav < av_health.size(); iav++)
    if (av_health[iav]->get_disease() == d && av_health[iav]->is_on_av(day))
      return true;
  return false;
}


void Health::infect(Person *infectee, int disease_id, Transmission *transmisison) {
  infection[disease_id]->transmit(infectee, transmisison);
  infectee_count[disease_id]++;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d infected person %d infectees = %d\n",
	    self->get_id(), infectee->get_id(), infectee_count[disease_id]);
    fflush(Global::Statusfp);
  }
}


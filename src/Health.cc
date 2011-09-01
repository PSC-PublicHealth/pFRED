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


Health::Health (Person * person) {
  self = person;
  infection = new Infection* [Global::Diseases];
  susceptibility_multp = new double [Global::Diseases];
  status = new char [Global::Diseases];
  susceptible = new bool [Global::Diseases];
  infectee_count = new int [Global::Diseases];
  symptomatic_status = false;

  for (int disease = 0; disease < Global::Diseases; disease++) {
    infection[disease] = NULL;
    infectee_count[disease] = 0;
    become_susceptible(disease);
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
  for(int disease = 0; disease < Global::Diseases; disease++) {
    Disease* s = Global::Pop.get_disease(disease);

    if(!s->get_at_risk()->is_empty()) {
      double at_risk_prob = s->get_at_risk()->find_value(self->get_age());

      if(RANDOM() < at_risk_prob) { // Now a probability <=1.0
        declare_at_risk(s);
      }
    }
  }
}

Health::~Health() {
  delete[] infection;
  delete[] susceptibility_multp;
  delete[] status;
  delete[] susceptible;
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

void Health::become_susceptible(int disease) {
  Disease * d = Global::Pop.get_disease(disease);
  if (infection[disease] != NULL) {
    delete infection[disease];
    infection[disease] = NULL;
  }
  susceptibility_multp[disease] = 1.0;
  d->become_susceptible(self);
  status[disease] = 'S';
  susceptible[disease] = true;
}

void Health::become_susceptible(Disease * disease) {
  int disease_id = disease->get_id();
  if (infection[disease_id] != NULL) {
    delete infection[disease_id];
    infection[disease_id] = NULL;
  }
  susceptibility_multp[disease_id] = 1.0;
  disease->become_susceptible(self);
  status[disease_id] = 'S';
  susceptible[disease_id] = true;
}

void Health::become_unsusceptible(Disease * disease) {
  int disease_id = disease->get_id();
  disease->become_unsusceptible(self);
  susceptible[disease_id] = false;
}


void Health::become_removed(int disease_id) {
  Disease * disease = Global::Pop.get_disease(disease_id);
  if (infection[disease_id] != NULL) {
    delete infection[disease_id];
    infection[disease_id] = NULL;
  }
  disease->become_removed(self, status[disease_id]);
  status[disease_id] = 'R';
  susceptible[disease_id] = false;
}

void Health::recover(Disease * disease) {
  int disease_id = disease->get_id();
  disease->become_removed(self, status[disease_id]);
  status[disease_id] = 'R';
  susceptible[disease_id] = false;
}

void Health::update(int day) {

  // if deceased, clear health status
  if (self->is_deceased()) {
    for (int d = 0; d < Global::Diseases; d++) {
      become_removed(d);
    }
    return;
  }

  // update vaccine status
  if (takes_vaccine) {
    int size = (int) vaccine_health.size();
    for (int i = 0; i < size; i++)
      vaccine_health[i]->update(day, self->get_age());
  }

  // update each infection
  symptomatic_status = false;
  for (int s = 0; s < Global::Diseases; s++) {
    if (status[s] == 'R' && infection[s] != NULL) {
      delete infection[s];
      infection[s] = NULL;
    }
    if (status[s] == 'S' || status[s] == 'M' || status[s] == 'R')
      continue;
    infection[s]->update(day);
    if (infection[s]->get_disease_status() == 'S')
      become_susceptible(s);
    if (status[s] == 'I')
      symptomatic_status = true;
  }

  // update antiviral status
  if (takes_av) {
    int size = av_health.size();
    for(int i = 0; i < size; i++)
      av_health[i]->update(day);
  }
}

void Health::become_infectious(Disease * disease) {
  int disease_id = disease->get_id();

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "INFECTIOUS person %d for disease %d\n",
            self->get_id(), disease_id);
    fflush(Global::Statusfp);
  }

  status[disease_id] = infection[disease_id]->get_disease_status();
  if (status[disease_id] == 'I')
    symptomatic_status = true;
  disease->become_infectious(self, status[disease_id]);

  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "INFECTIOUS person %d for disease %d has status %c %s\n",
            self->get_id(), disease_id,
            status[disease_id], symptomatic_status?"symptomatic":"asymptomatic");
    fflush(Global::Statusfp);
  }
}

void Health::become_symptomatic(Disease *disease) {
  int disease_id = disease->get_id();
  assert(infection[disease_id] != NULL);
  char old_status = status[disease_id];
  status[disease_id] = 'I';
  symptomatic_status = true;
  disease->become_symptomatic(self, old_status);
}


void Health::become_immune(Disease* disease) {
  int disease_id = disease->get_id();
  immunity[disease_id] = true;
  disease->become_immune(self, status[disease_id]);
  status[disease_id] = 'M';
  susceptible[disease_id] = false;
}

void Health::declare_at_risk(Disease* disease) {
  int disease_id = disease->get_id();
  at_risk[disease_id] = true;
}

bool Health::is_symptomatic() const {
  for (int disease = 0; disease < Global::Diseases; disease++) {
    if (infection[disease] != NULL && infection[disease]->is_symptomatic())
      return true;
  }
  return false;
}

int Health::get_exposure_date(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else
    return infection[disease]->get_exposure_date();
}

int Health::get_infectious_date(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else
    return infection[disease]->get_infectious_date();
}

int Health::get_recovered_date(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else
    return infection[disease]->get_recovery_date();
}

int Health:: get_symptomatic_date(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else
    return infection[disease]->get_symptomatic_date();
}

int Health::get_infector(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else if (infection[disease]->get_infector() == NULL)
    return -1;
  else
    return infection[disease]->get_infector()->get_id();
}

int Health::get_infected_place(int disease) const {
  if (infection[disease] == NULL)
    return -1;
  else if (infection[disease]->get_infected_place() == NULL)
    return -1;
  else
    return infection[disease]->get_infected_place()->get_id();
}

char Health::get_infected_place_type(int disease) const {
  if (infection[disease] == NULL)
    return 'X';
  else if (infection[disease]->get_infected_place() == NULL)
    return 'X';
  else
    return infection[disease]->get_infected_place()->get_type();
}

char * Health::get_infected_place_label(int disease) const {
  if (infection[disease] == NULL) {
    strcpy(dummy_label, "-");
    return dummy_label;
  } else if (infection[disease]->get_infected_place() == NULL) {
    strcpy(dummy_label, "X");
    return dummy_label;
  } else
    return infection[disease]->get_infected_place()->get_label();
}

int Health::get_infectees(int disease) const {
  return infectee_count[disease];
}

double Health::get_susceptibility(int disease) const {
  double suscep_multp = susceptibility_multp[disease];

  if (infection[disease] == NULL)
    return suscep_multp;
  else
    return infection[disease]->get_susceptibility() * suscep_multp;
}

double Health::get_infectivity(int disease, int day) const {
  if (infection[disease] == NULL)
    return 0.0;
  else
    return infection[disease]->get_infectivity(day);
}

//Modify Operators
void Health::modify_susceptibility(int disease, double multp) {
  if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " susceptibility for disease "
                               << disease << " by " << multp << "\n";

  susceptibility_multp[disease] *= multp;
}

void Health::modify_infectivity(int disease, double multp) {
  if (infection[disease] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease
                                 << " by " << multp << "\n";

    infection[disease]->modify_infectivity(multp);
  }
}

void Health::modify_infectious_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease
                                 << " by " << multp << "\n";

    infection[disease]->modify_infectious_period(multp, cur_day);
  }
}

void Health::modify_asymptomatic_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " asymptomatic period  for disease " << disease
                                 << " by " << multp << "\n";

    infection[disease]->modify_asymptomatic_period(multp, cur_day);
  }
}

void Health::modify_symptomatic_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomatic period  for disease " << disease
                                 << " by " << multp << "\n";

    infection[disease]->modify_symptomatic_period(multp, cur_day);
  }
}

void Health::modify_develops_symptoms(int disease, bool symptoms, int cur_day) {
  if (infection[disease] != NULL &&
      (infection[disease]->get_disease_status() == 'i' ||
       infection[disease]->get_disease_status() == 'E')) {
    if(Global::Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomaticity  for disease " << disease
                                 << " to " << symptoms << "\n";

    infection[disease]->modify_develops_symptoms(symptoms, cur_day);
    status[disease] = 'I';
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

bool Health::is_on_av_for_disease(int day, int s) const {
  for (unsigned int iav = 0; iav < av_health.size(); iav++)
    if (av_health[iav]->get_disease() == s && av_health[iav]->is_on_av(day))
      return true;

  return false;
}


void Health::infect(Person *infectee, int disease, Transmission *transmisison) {
  infection[disease]->transmit(infectee, transmisison);
  infectee_count[disease]++;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "person %d infected person %d infectees = %d\n",
	    self->get_id(), infectee->get_id(), infectee_count[disease]);
    fflush(Global::Statusfp);
  }
}

void Health::getInfected(Disease *disease, Transmission *transmission) {
  if (Global::Verbose > 1) {
    if (!transmission->getInfectedPlace())
      fprintf(Global::Statusfp, "EXPOSED_DUMMY person %d to disease %d\n",
	      self->get_id(), disease->get_id());
    else
      fprintf(Global::Statusfp, "EXPOSED person %d to disease %d\n",
	      self->get_id(), disease->get_id());
  }
  int diseaseID = disease->get_id();
  status[diseaseID] = 'E';

  if(infection[diseaseID] == NULL) {
    infection[diseaseID] = new Infection(disease, transmission->getInfector(), this->get_self(),
					 transmission->getInfectedPlace(), transmission->get_exposure_date());
  }

  Infection *infection_ptr = infection[diseaseID];
  infection_ptr->addTransmission(transmission);
  disease->become_exposed(self);
}


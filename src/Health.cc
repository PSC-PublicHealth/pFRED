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
  diseases = self->get_population()->get_diseases();

  infection = new Infection* [diseases];
  status = new char [diseases];
  susceptible = new bool [diseases];

  for (int disease = 0; disease < diseases; disease++) {
    infection[disease] = NULL;
    status[disease] = 'S';
    susceptible[disease] = true;
    }

  susceptibility_multp = new double [diseases];

  if(nantivirals == -1) {
    get_param_from_string("number_antivirals",&nantivirals);
    }

  checked_for_av.assign(nantivirals,false);
  immunity.assign(diseases,false);
  at_risk.assign(diseases,false);
  // reset();
  }

Health::~Health() {
  if( infection != NULL ) {
    delete[] infection;
    infection=NULL;
    }

  for(unsigned int i=0; i<vaccine_health.size(); i++)
    delete vaccine_health[i];

  vaccine_health.clear();
  takes_vaccine = false;

  for(unsigned int i=0; i<av_health.size(); i++)
    delete av_health[i];

  av_health.clear();
  takes_av = false;
  }

void Health::reset() {
  // printf("reset health for person %d  diseases = %d\n",self->get_id(),diseases); fflush(stdout);
  symptomatic_status = false;
  immunity.assign(immunity.size(),false);
  at_risk.assign(at_risk.size(),false);

  for (int disease = 0; disease < diseases; disease++) {
    become_susceptible(disease);
    }

  //Clean out the stats objects
  for(unsigned int i=0; i<vaccine_health.size(); i++)
    delete vaccine_health[i];

  vaccine_health.clear();
  takes_vaccine = false;

  for(unsigned int i=0; i<av_health.size(); i++)
    delete av_health[i];

  av_health.clear();
  takes_av = false;

  checked_for_av.assign(checked_for_av.size(),false);

  // Determine if the agent is at risk
  for(int disease = 0; disease < diseases; disease++) {
    Disease* s = self->get_population()->get_disease(disease);

    if(!s->get_at_risk()->is_empty()) {
      double at_risk_prob = s->get_at_risk()->find_value(self->get_age());

      if(RANDOM() < at_risk_prob) { // Now a probability <=1.0
        declare_at_risk(s);
        }
      }
    }
  }

void Health::become_susceptible(int disease) {
  if (infection[disease] != NULL) {
    delete infection[disease];
    infection[disease] = NULL;
    }

  susceptibility_multp[disease] = 1.0;
  self->get_population()->get_disease(disease)->increment_S_count();
  status[disease] = 'S';
  susceptible[disease] = true;
  }

void Health::become_unsusceptible(int disease) {
  if(infection[disease] != NULL) infection[disease]->become_unsusceptible();

  susceptible[disease] = false;
  }


void Health::become_removed(int disease) {
  if (infection[disease] != NULL) {
    infection[disease]->remove();
    delete infection[disease];
    infection[disease] = NULL;
    }
  else {
    self->get_population()->get_disease(disease)->decrement_S_count();
    }

  status[disease] = 'D';
  }

void Health::update(int day) {

  // if deceased, clear health status
  if (self->is_deceased()) {
    for (int s = 0; s < diseases; s++) {
      become_removed(s);
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

  for (int s = 0; s < diseases; s++) {
    if (status[s] == 'S' || status[s] == 'M')
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

void Health::become_exposed(Infection * infection_ptr) {
  Disease * disease = infection_ptr->get_disease();
  int disease_id = disease->get_id();

  if (Verbose > 1) {
    if (!infection_ptr->get_infected_place())
      fprintf(Statusfp, "EXPOSED_DUMMY person %d to disease %d\n", self->get_id(), disease_id);
    else
      fprintf(Statusfp, "EXPOSED person %d to disease %d\n", self->get_id(), disease_id);
    }

  if (infection[disease_id] != NULL) {
    assert(infection[disease_id]->get_disease_status() == 'R');
    delete infection[disease_id];
    infection[disease_id] = infection_ptr;
    }
  else {
    infection[disease_id] = infection_ptr;
    }

  status[disease_id] = 'E';

  if (All_diseases_antigenically_identical) {
    // HACK - this is probably NOT how we want to do this.  But disease
    // representation/mutation is still a very open problem for us.
    // become immune to equivalent diseases.
    for (int i = 0; i < diseases; i++) {
      if (infection[i] == NULL) {
        // Could also check some sort of antigenic status here, but for now,
        // just assume all diseases are antigenically identical.
        Disease* s = Pop.get_disease(i);
        Infection* dummy_i = Infection::get_dummy_infection(s, self, infection_ptr->get_exposure_date());
        become_exposed(dummy_i);
        become_infectious(s);
        recover(s);
        }
      }
    }
  }

void Health::become_infectious(Disease * disease) {
  int disease_id = disease->get_id();

  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for disease %d is %s\n",
            self->get_id(), disease_id, symptomatic_status?"symptomatic":"asymptomatic");
    fflush(Statusfp);
    }

  infection[disease_id]->become_infectious();
  disease->insert_into_infectious_list(self);
  status[disease_id] = infection[disease_id]->get_disease_status();

  if (status[disease_id] == 'I')
    symptomatic_status = true;

  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for disease %d has status %c %s\n",
            self->get_id(), disease_id,
            status[disease_id], symptomatic_status?"symptomatic":"asymptomatic");
    fflush(Statusfp);
    }
  }

void Health::become_symptomatic(Disease *disease) {
  int disease_id = disease->get_id();

  if (!infection[disease_id])
    throw logic_error("become symptomatic: cannot, without being exposed first");

  infection[disease->get_id()]->become_symptomatic();
  status[disease_id] = 'I';
  symptomatic_status = true;
  }


void Health::become_immune(Disease* disease) {
  int disease_id = disease->get_id();

  if(status[disease_id] == 'S') {
    immunity[disease_id] = true;
    disease->increment_M_count();
    status[disease_id] = 'M';
    susceptible[disease_id] = false;
    }
  }

void Health::declare_at_risk(Disease* disease) {
  int disease_id = disease->get_id();
  at_risk[disease_id] = true;
  }

void Health::recover(Disease * disease) {
  int disease_id = disease->get_id();
  infection[disease_id]->recover();
  disease->remove_from_infectious_list(self);
  status[disease_id] = 'R';
  susceptible[disease_id] = false;
  }

int Health::is_symptomatic() const {
  for (int disease = 0; disease < diseases; disease++) {
    if (infection[disease] != NULL && infection[disease]->is_symptomatic())
      return 1;
    }

  return 0;
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
    }
  else if (infection[disease]->get_infected_place() == NULL) {
    strcpy(dummy_label, "X");
    return dummy_label;
    }
  else
    return infection[disease]->get_infected_place()->get_label();
  }

int Health::get_infectees(int disease) const {
  if (infection[disease] == NULL)
    return 0;
  else
    return infection[disease]->get_infectee_count();
  }

int Health::add_infectee(int disease) {
  if (infection[disease] == NULL)
    return 0;
  else
    return infection[disease]->add_infectee();
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
  if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " susceptibility for disease "
                       << disease << " by " << multp << "\n";

  susceptibility_multp[disease] *= multp;
  }

void Health::modify_infectivity(int disease, double multp) {
  if (infection[disease] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease
                         << " by " << multp << "\n";

    infection[disease]->modify_infectivity(multp);
    }
  }

void Health::modify_infectious_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for disease " << disease
                         << " by " << multp << "\n";

    infection[disease]->modify_infectious_period(multp, cur_day);
    }
  }

void Health::modify_asymptomatic_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " asymptomatic period  for disease " << disease
                         << " by " << multp << "\n";

    infection[disease]->modify_asymptomatic_period(multp, cur_day);
    }
  }

void Health::modify_symptomatic_period(int disease, double multp, int cur_day) {
  if (infection[disease] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomatic period  for disease " << disease
                         << " by " << multp << "\n";

    infection[disease]->modify_symptomatic_period(multp, cur_day);
    }
  }

void Health::modify_develops_symptoms(int disease, bool symptoms, int cur_day) {
  if (infection[disease] != NULL &&
      (infection[disease]->get_disease_status() == 'i' ||
       infection[disease]->get_disease_status() == 'E')) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomaticity  for disease " << disease
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
    }
  else { // Already have a dose, need to take the next dose
    vaccine_health_for_dose->update_for_next_dose(day,age);
    }

  if (VaccineTracefp != NULL) {
    fprintf(VaccineTracefp," id %7d vaccid %3d",self->get_id(),vaccine_health[vaccine_health.size()-1]->get_vaccine()->get_ID());
    vaccine_health[vaccine_health.size()-1]->printTrace();
    fprintf(VaccineTracefp,"\n");
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
  }

void Health::getInfected(Disease *disease, Transmission *transmission) {
  if (Verbose > 1) {
    if (!transmission->getInfectedPlace())
      fprintf(Statusfp, "EXPOSED_DUMMY person %d to disease %d\n", self->get_id(), disease->get_id());
    else
      fprintf(Statusfp, "EXPOSED person %d to disease %d\n", self->get_id(), disease->get_id());
    }

  int diseaseID = disease->get_id();
  status[diseaseID] = 'E';

  if(infection[diseaseID] == NULL) {
    infection[diseaseID] = new Infection(disease, transmission->getInfector(), this->get_self(), transmission->getInfectedPlace(), transmission->get_exposure_date());
    }

  Infection *infection_ptr = infection[diseaseID];
  infection_ptr->addTransmission(transmission);
  }

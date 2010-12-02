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

Health::Health (Person * person) {
  self = person;
  diseases = self->get_population()->get_diseases();
  
  infection = new Infection* [diseases];
  status = new char [diseases];
  for (int disease = 0; disease < diseases; disease++) {
    infection[disease] = NULL;
    status[disease] = 'S';
  }
  susceptibility_multp = new double [diseases];
  
  // Immunity to the diseases setting immunity to 0 
  int nav;
  //get_param((char *) "number_antivirals",&nav);
  nav = 4;  // Just put there to make sure there is space.  
  // If  I call get_param, it prints out a line in the output 
  // for every agent.  Need to fix.
  //This actually allocates the space for these vectors
  checked_for_av.assign(nav,false);
  immunity.assign(diseases,false);
  at_risk.assign(diseases,false);
  reset();
}

Health::~Health(){
  if( infection != NULL ){
    delete[] infection;
    infection=NULL;
  }
  for(unsigned int i=0;i<vaccine_health.size();i++)
    delete vaccine_health[i];
  vaccine_health.clear();
  takes_vaccine = false;

  for(unsigned int i=0;i<av_health.size();i++)
    delete av_health[i];
  av_health.clear();
  takes_av = false;
}

void Health::reset() {
  immunity.assign(immunity.size(),false);
  at_risk.assign(at_risk.size(),false);
  for (int disease = 0; disease < diseases; disease++) {
    become_susceptible(disease);
  }
  
  //Clean out the stats objects
  for(unsigned int i=0;i<vaccine_health.size();i++)
    delete vaccine_health[i];
  vaccine_health.clear();
  takes_vaccine = false;
  
  for(unsigned int i=0;i<av_health.size();i++)
    delete av_health[i];
  av_health.clear();
  takes_av = false;
  
  checked_for_av.assign(checked_for_av.size(),false);
  
  // Determine if the agent is at risk
  for(int disease = 0; disease < diseases; disease++) {
    Disease* s = self->get_population()->get_disease(disease);
    if(!s->get_at_risk()->is_empty()){
      double at_risk_prob = s->get_at_risk()->find_value(self->get_age());
      if(RANDOM() < at_risk_prob){ // Now a probability <=1.0
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
}

void Health::update(int day) {
  // update vaccine status
  if (takes_vaccine) {
    int size = (int) vaccine_health.size();
    for (int i = 0; i < size; i++)
      vaccine_health[i]->update(day, self->get_age());
  }

  // possibly mutate infections
  if (takes_av) {
    update_mutations(day);
  }
  
  // update each infection
  for (int s = 0; s < diseases; s++) {
    if (status[s] == 'S' || status[s] == 'M')
      continue;
    infection[s]->update(day);
    if (infection[s]->get_disease_status() == 'S')
      self->become_susceptible(s);
  }
  
  // update antiviral status
  if (takes_av) {
    int size = av_health.size();
    for(int i = 0; i < size; i++)
      av_health[i]->update(day);
  }
}

void Health::update_mutations(int day) {
  // Possibly mutate each active disease
  // TODO(alona) should consider diseases in random order, but that
  // is not relevant right now (only 1 or 2 diseases)
  // First, update vaccine status
  for (int s = 0; s < diseases; s++) {
    if (status[s] == 'S' || status[s] == 'M' || status[s] == 'R')
      continue;
    if (is_on_av_for_disease(day, s) == true) {
      if (Verbose > 1) {
        fprintf(Statusfp, "person %d IS ON AV on day %i\n", self->get_id(), day-1);
        fflush(Statusfp);
      }
      if (infection[s]->possibly_mutate(this, day)) {
        if (Verbose > 1) {
          fprintf(Statusfp, "person %d disease %d mutated on day %i\n",
                  self->get_id(), s, day);
          fflush(Statusfp);
        }
        break;
      }
    }
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
  } else {
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
        Infection* dummy_i =
	  Infection::get_dummy_infection(s, self, infection_ptr->get_exposure_date());
        self->become_exposed(dummy_i);
        self->become_infectious(s);
        self->recover(s);
      }
    }
  }
}

void Health::become_infectious(Disease * disease) {
  int disease_id = disease->get_id();
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for disease %d %d\n", self->get_id(), disease_id, is_symptomatic());
    fflush(Statusfp);
  }
  infection[disease_id]->become_infectious();
  disease->insert_into_infectious_list(self);
  status[disease_id] = infection[disease_id]->get_disease_status();
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for disease %d has status %c %d\n",
            self->get_id(), disease_id, status[disease_id], is_symptomatic());
    fflush(Statusfp);
  }
}

void Health::become_symptomatic(Disease *disease) {
  int disease_id = disease->get_id();
  if (!infection[disease_id])
    throw logic_error("become symptomatic: cannot, without being exposed first");
  infection[disease->get_id()]->become_symptomatic();
  status[disease_id] = 'I';
}


void Health::become_immune(Disease* disease) {
  int disease_id = disease->get_id();
  if(status[disease_id] == 'S'){
    immunity[disease_id] = true; 
    disease->increment_M_count();
    status[disease_id] = 'M';
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

double Health::get_infectivity(int disease) const {
  if (infection[disease] == NULL)
    return 0.0;
  else
    return infection[disease]->get_infectivity();
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

void Health::modify_infectious_period(int disease, double multp, int cur_day){
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
       infection[disease]->get_disease_status() == 'E')){
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomaticity  for disease " << disease 
		       << " to " << symptoms << "\n";
    infection[disease]->modify_develops_symptoms(symptoms, cur_day);
    status[disease] = 'I';
  }
}

//Medication operators
void Health::take(Vaccine* vaccine, int day){
  // Compliance will be somewhere else
  int age = self->get_age();
  vaccine_health.push_back(new Vaccine_Health(day,vaccine,age,this));
  takes_vaccine = true;
  
  if (VaccineTracefp != NULL) {
    fprintf(VaccineTracefp," id %7d vaccid %3d",self->get_id(),vaccine_health[vaccine_health.size()-1]->get_vaccine()->get_ID());
    vaccine_health[vaccine_health.size()-1]->printTrace();
    fprintf(VaccineTracefp,"\n");
  }

  return;
}

void Health::take(Antiviral* av, int day){
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


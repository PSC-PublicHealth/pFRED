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
#include "Strain.h"
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
  strains = self->get_population()->get_strains();
  
  infection = new Infection* [strains];
  status = new char [strains];
  for (int strain = 0; strain < strains; strain++) {
    infection[strain] = NULL;
    status[strain] = 'S';
  }
  susceptibility_multp = new double [strains];
  
  // Immunity to the strains setting immunity to 0 
  int nav;
  //get_param((char *) "number_antivirals",&nav);
  nav = 4;  // Just put there to make sure there is space.  
  // If  I call get_param, it prints out a line in the output 
  // for every agent.  Need to fix.
  //This actually allocates the space for these vectors
  checked_for_av.assign(nav,false);
  immunity.assign(strains,false);
  at_risk.assign(strains,false);
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
  for (int strain = 0; strain < strains; strain++) {
    become_susceptible(strain);
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
  for(int strain = 0; strain < strains; strain++) {
    Strain* s = self->get_population()->get_strain(strain);
    if(!s->get_at_risk()->is_empty()){
      double at_risk_prob = s->get_at_risk()->find_value(self->get_age());
      if(RANDOM() < at_risk_prob){ // Now a probability <=1.0
        declare_at_risk(s);
      }
    }
  }
}

void Health::become_susceptible(int strain) {
  if (infection[strain] != NULL) {
    delete infection[strain];
    infection[strain] = NULL;
  }
  susceptibility_multp[strain] = 1.0;
  self->get_population()->get_strain(strain)->increment_S_count();
  status[strain] = 'S';
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
  for (int s = 0; s < strains; s++) {
    if (status[s] == 'S' || status[s] == 'M')
      continue;
    infection[s]->update(day);
    if (infection[s]->get_strain_status() == 'S')
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
  // Possibly mutate each active strain
  // TODO(alona) should consider strains in random order, but that
  // is not relevant right now (only 1 or 2 strains)
  // First, update vaccine status
  for (int s = 0; s < strains; s++) {
    if (status[s] == 'S' || status[s] == 'M' || status[s] == 'R')
      continue;
    if (is_on_av_for_strain(day, s) == true) {
      if (Verbose > 1) {
        fprintf(Statusfp, "person %d IS ON AV on day %i\n", self->get_id(), day-1);
        fflush(Statusfp);
      }
      if (infection[s]->possibly_mutate(this, day)) {
        if (Verbose > 1) {
          fprintf(Statusfp, "person %d strain %d mutated on day %i\n",
                  self->get_id(), s, day);
          fflush(Statusfp);
        }
        break;
      }
    }
  }
}


void Health::become_exposed(Infection * infection_ptr) {
  Strain * strain = infection_ptr->get_strain();
  int strain_id = strain->get_id();
  if (Verbose > 1) {
    if (!infection_ptr->get_infected_place())
      fprintf(Statusfp, "EXPOSED_DUMMY person %d to strain %d\n", self->get_id(), strain_id);
    else
      fprintf(Statusfp, "EXPOSED person %d to strain %d\n", self->get_id(), strain_id);
  }
  if (infection[strain_id] != NULL) {
    assert(infection[strain_id]->get_strain_status() == 'R');
    delete infection[strain_id];
    infection[strain_id] = infection_ptr;
  } else {
    infection[strain_id] = infection_ptr;
  }
  status[strain_id] = 'E';
  
  if (All_strains_antigenically_identical) {
    // HACK - this is probably NOT how we want to do this.  But strain
    // representation/mutation is still a very open problem for us.
    // become immune to equivalent strains.
    for (int i = 0; i < strains; i++) {
      if (infection[i] == NULL) {
        // Could also check some sort of antigenic status here, but for now,
        // just assume all strains are antigenically identical.
        Strain* s = Pop.get_strain(i);
        Infection* dummy_i =
	  Infection::get_dummy_infection(s, self, infection_ptr->get_exposure_date());
        self->become_exposed(dummy_i);
        self->become_infectious(s);
        self->recover(s);
      }
    }
  }
}

void Health::become_infectious(Strain * strain) {
  int strain_id = strain->get_id();
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d %d\n", self->get_id(), strain_id, is_symptomatic());
    fflush(Statusfp);
  }
  infection[strain_id]->become_infectious();
  strain->insert_into_infectious_list(self);
  status[strain_id] = infection[strain_id]->get_strain_status();
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d has status %c %d\n",
            self->get_id(), strain_id, status[strain_id], is_symptomatic());
    fflush(Statusfp);
  }
}

void Health::become_symptomatic(Strain *strain) {
  int strain_id = strain->get_id();
  if (!infection[strain_id])
    throw logic_error("become symptomatic: cannot, without being exposed first");
  infection[strain->get_id()]->become_symptomatic();
  status[strain_id] = 'I';
}


void Health::become_immune(Strain* strain) {
  int strain_id = strain->get_id();
  if(status[strain_id] == 'S'){
    immunity[strain_id] = true; 
    strain->increment_M_count();
    status[strain_id] = 'M';
  }
}

void Health::declare_at_risk(Strain* strain) {
  int strain_id = strain->get_id();
  at_risk[strain_id] = true;
}

void Health::recover(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id]->recover();
  strain->remove_from_infectious_list(self);
  status[strain_id] = 'R';
}

int Health::is_symptomatic() const {
  for (int strain = 0; strain < strains; strain++) {
    if (infection[strain] != NULL && infection[strain]->is_symptomatic())
      return 1;
  }
  return 0;
}

int Health::get_exposure_date(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_exposure_date();
}

int Health::get_infectious_date(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_infectious_date();
}

int Health::get_recovered_date(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_recovery_date();
}

int Health:: get_symptomatic_date(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_symptomatic_date();
}
int Health::get_infector(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else if (infection[strain]->get_infector() == NULL)
    return -1;
  else
    return infection[strain]->get_infector()->get_id();
}

int Health::get_infected_place(int strain) const {
  if (infection[strain] == NULL) 
    return -1;
  else if (infection[strain]->get_infected_place() == NULL)
    return -1;
  else
    return infection[strain]->get_infected_place()->get_id();
}

char Health::get_infected_place_type(int strain) const {
  if (infection[strain] == NULL) 
    return 'X';
  else if (infection[strain]->get_infected_place() == NULL)
    return 'X';
  else
    return infection[strain]->get_infected_place()->get_type();
}

int Health::get_infectees(int strain) const {
  if (infection[strain] == NULL) 
    return 0;
  else
    return infection[strain]->get_infectee_count();
}

int Health::add_infectee(int strain) {
  if (infection[strain] == NULL)
    return 0;
  else
    return infection[strain]->add_infectee();
}

double Health::get_susceptibility(int strain) const {
  double suscep_multp = susceptibility_multp[strain];
  if (infection[strain] == NULL)
    return suscep_multp;
  else
    return infection[strain]->get_susceptibility() * suscep_multp;
}

double Health::get_infectivity(int strain) const {
  if (infection[strain] == NULL)
    return 0.0;
  else
    return infection[strain]->get_infectivity();
}

//Modify Operators
void Health::modify_susceptibility(int strain, double multp) {
  if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " susceptibility for strain " 
		     << strain << " by " << multp << "\n";
  susceptibility_multp[strain] *= multp;
}

void Health::modify_infectivity(int strain, double multp) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for strain " << strain 
		       << " by " << multp << "\n";
    infection[strain]->modify_infectivity(multp);
  }
}

void Health::modify_infectious_period(int strain, double multp, int cur_day){
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " infectivity for strain " << strain 
		       << " by " << multp << "\n";
    infection[strain]->modify_infectious_period(multp, cur_day);
  }
}

void Health::modify_asymptomatic_period(int strain, double multp, int cur_day) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " asymptomatic period  for strain " << strain 
		       << " by " << multp << "\n";
    infection[strain]->modify_asymptomatic_period(multp, cur_day);
  }
}

void Health::modify_symptomatic_period(int strain, double multp, int cur_day) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomatic period  for strain " << strain 
		       << " by " << multp << "\n";
    infection[strain]->modify_symptomatic_period(multp, cur_day);
  }
}

void Health::modify_develops_symptoms(int strain, bool symptoms, int cur_day) {
  if (infection[strain] != NULL &&
      (infection[strain]->get_strain_status() == 'i' ||
       infection[strain]->get_strain_status() == 'E')){
    if(Debug > 2) cout << "Modifying Agent " << self->get_id() << " symptomaticity  for strain " << strain 
		       << " to " << symptoms << "\n";
    infection[strain]->modify_develops_symptoms(symptoms, cur_day);
    status[strain] = 'I';
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

bool Health::is_on_av_for_strain(int day, int s) const {
  for (unsigned int iav = 0; iav < av_health.size(); iav++)
    if (av_health[iav]->get_strain() == s && av_health[iav]->is_on_av(day))
      return true;
  return false;
}


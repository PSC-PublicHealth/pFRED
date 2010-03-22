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
#include "Health.h"
#include "Person.h"
#include "Strain.h"
#include "Infection.h"
#include "Antiviral.h"
#include "Population.h"
#include "Random.h"
#include "Global.h"
#include "Manager.h"
#include "AV_Manager.h"
#include "AV_Status.h"
#include "Vaccine.h"
#include "VaccineDose.h"
#include "VaccineStatus.h"

Health::Health (Person * person) {
  self = person;
  strains = Pop.get_strains();

  infection = new Infection * [strains];
  for (int strain = 0; strain < strains; strain++) {
    infection[strain] = NULL;
  }
  immunity.assign(strains,0);
  susceptibility_multp = new double [strains];

  // Immunity to the strains setting immunity to 0 
  immunity.assign(strains,0);
  checked_for_av.assign(strains,0);
  reset();
}

Health::~Health(void){
  delete infection;
  for(unsigned int i=0;i<vaccine_stats.size();i++)
    delete vaccine_stats[i];
  for(unsigned int i=0;i<av_stats.size();i++)
    delete av_stats[i];
}

void Health::reset() {

  for (int strain = 0; strain < strains; strain++) {
    immunity[strain] = 0;
    become_susceptible(strain);
  }
  immunity.assign(immunity.size(),0);
  checked_for_av.assign(checked_for_av.size(),0);
}

void Health::become_susceptible(int strain) {
  if (infection[strain] != NULL) {
    delete infection[strain];
  }
  infection[strain] = NULL;
  susceptibility_multp[strain] = 1.0;
}

void Health::update(int day) {
  //Update Vaccine Status
  for(unsigned int i=0;i<vaccine_stats.size();i++) vaccine_stats[i]->update(day, self->get_age());
    
  update_mutations(day);
  for (int s = 0; s < strains; s++) {
    char status = self->get_strain_status(s);
    if (status == 'S' || status == 'M')
      continue;
    infection[s]->update(day);
  }
  
  //Update Antiviral Status
  for(unsigned int i=0;i<av_stats.size();i++) av_stats[i]->update(day);
}

void Health::update_mutations(int day) {
  // Possibly mutate each active strain
  // TODO(alona) should consider strains in random order, but that
  // is not relevant right now (only 1 or 2 strains)
  // First, update vaccine status
  for (int s = 0; s < strains; s++) {
    char status = get_strain_status(s);
    if (status == 'S' || status == 'M' || status == 'R')
      continue;
    if (is_on_av(day, s) != -1) {
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
    if (infection_ptr->get_infected_place_id() == -1)
      fprintf(Statusfp, "EXPOSED_DUMMY person %d to strain %d\n", self->get_id(), strain_id);
    else
      fprintf(Statusfp, "EXPOSED person %d to strain %d\n", self->get_id(), strain_id);
  }
  if (infection[strain_id] != NULL) {
    if (infection[strain_id]->get_strain_status() != 'R') {
      printf("WARNING! Attempted double exposure for strain %i in person %i. "
	     "Current status is %c "
	     "Double infection is not supported.\n",
	     strain_id, self->get_id(), infection[strain_id]->get_strain_status());
      abort();
    }
    delete infection[strain_id];
    infection[strain_id] = infection_ptr;
  } else {
    infection[strain_id] = infection_ptr;
  }
  strain->insert_into_exposed_list(self);

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
  strain->remove_from_exposed_list(self);
  strain->insert_into_infectious_list(self);
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d has status %c %d\n",
	    self->get_id(), strain_id, get_strain_status(strain_id),is_symptomatic());
    fflush(Statusfp);
  }
}

void Health::recover(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id]->recover();
  strain->remove_from_infectious_list(self);
}

void Health::immunize(Strain* strain){
  int strain_id = strain->get_id();
  if(get_strain_status(strain_id) == 'S'){
    immunity[strain_id] = 1;
  }
}

int Health::is_symptomatic() {
  for (int strain = 0; strain < strains; strain++) {
    if (infection[strain] != NULL && infection[strain]->is_symptomatic())
      return 1;
  }
  return 0;
}

int Health::get_exposure_date(int strain) {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_exposure_date();
}

int Health::get_infectious_date(int strain) {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_infectious_date();
}

int Health::get_recovered_date(int strain) {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_recovered_date();
}

int Health::get_infector(int strain) {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_infector();
}

int Health::get_infected_place(int strain) {
  if (infection[strain] == NULL) 
    return -1;
  else
    return infection[strain]->get_infected_place_id();
}

char Health::get_infected_place_type(int strain) {
  if (infection[strain] == NULL) 
    return 'X';
  else
    return infection[strain]->get_infected_place_type();
}

int Health::get_infectees(int strain) {
  if (infection[strain] == NULL) 
    return 0;
  else
    return infection[strain]->get_infectees();
}

int Health::add_infectee(int strain) {
  if (infection[strain] == NULL)
    return 0;
  else
    return infection[strain]->add_infectee();
}

double Health::get_susceptibility(int strain) {
  double suscep_multp = susceptibility_multp[strain];
  if (infection[strain] == NULL)
    return suscep_multp;
  else
    return infection[strain]->get_susceptibility() * suscep_multp;
}

double Health::get_infectivity(int strain) {
  if (infection[strain] == NULL)
    return 0.0;
  else
    return infection[strain]->get_infectivity();
}


//Modify Operators
void Health::modify_susceptibility(int strain, double multp) {
  if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " susceptibility for strain " << strain << " by " << multp;
  susceptibility_multp[strain] *= multp;
}

void Health::modify_infectivity(int strain, double multp) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " infectivity for strain " << strain << " by " << multp;
    infection[strain]->modify_infectivity(multp);
  }
}

void Health::modify_infectious_period(int strain, double multp, int cur_day){
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " infectivity for strain " << strain << " by " << multp;
    infection[strain]->modify_infectious_period(multp, cur_day);
  }
}

void Health::modify_asymptomatic_period(int strain, double multp, int cur_day) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " asymptomatic period  for strain " << strain << " by " << multp;
    infection[strain]->modify_asymptomatic_period(multp, cur_day);
  }
}

void Health::modify_symptomatic_period(int strain, double multp, int cur_day) {
  if (infection[strain] != NULL) {
    if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " symptomatic period  for strain " << strain << " by " << multp;
    infection[strain]->modify_symptomatic_period(multp, cur_day);
  }
}

void Health::modify_develops_symptoms(int strain, bool symptoms, int cur_day) {
  if (infection[strain] != NULL &&
      (infection[strain]->get_strain_status() == 'i' ||
       infection[strain]->get_strain_status() == 'E')){
    if(Debug > 2) cout << "\nModifying Agent " << self->get_id() << " symptomaticity  for strain " << strain << " to " << symptoms;
    infection[strain]->modify_develops_symptoms(symptoms, cur_day);
  }
}

void Health::immunize(int strain){
  if(get_strain_status(strain) == 'S'){
    immunity[strain] = 1;
  }
}
  
//Medication operators
int Health::take(Antiviral *av, int day){
  av_stats.push_back(new AV_Status(day,av,this));
  return 1;
}

int Health::take(Vaccine *vacc, int day){
  // Compliance will be somewhere else
  int age = self->get_age();
  vaccine_stats.push_back(new Vaccine_Status(day,vacc,age,this));
  
  fprintf(VaccineTracefp," id %7d vaccid %3d ",self->get_id(),vaccine_stats[vaccine_stats.size()-1]->get_vaccine()->get_ID());
  vaccine_stats[vaccine_stats.size()-1]->printTrace();
  fprintf(VaccineTracefp,"\n");
  return 0;
}

int Health::is_on_av(int day, int s){
  for(unsigned int iav = 0; iav < av_stats.size(); iav++){
    if(av_stats[iav]->get_strain()==s &&
       (day >= av_stats[iav]->get_av_start_day() &&
	day <= av_stats[iav]->get_av_end_day())){
      return 1;
    }
  }
  return -1;
}

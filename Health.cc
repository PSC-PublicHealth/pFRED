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

Health::Health (Person * person) {
  self = person;
  strains = Pop.get_strains();
  AVs = Pop.get_AVs();
  infection = new Infection * [strains];
  for (int strain = 0; strain < strains; strain++) {
    infection[strain] = NULL;
  }
  susceptibility_multp = new double [strains];
  reset();
}

void Health::reset() {
  number_av_taken=0;
  for (int strain = 0; strain < strains; strain++) {
    become_susceptible(strain);
  }
}

void Health::become_susceptible(int strain) {
  if (infection[strain] != NULL) {
    delete infection[strain];
  }
  infection[strain] = NULL;
  susceptibility_multp[strain] = 1.0;
}

void Health::update(int day) {
  update_mutations(day);
  for (int s = 0; s < strains; s++) {
    char status = self->get_strain_status(s);
    if (status == 'S')
      continue;
    infection[s]->update(day);
  }
  update_antivirals(day);
}

void Health::update_mutations(int day) {
  //  AVs->print();
  // Possibly mutate each active strain
  // TODO(alona) should consider strains in random order, but that
  // is not relevant right now (only 1 or 2 strains)
  for (int s = 0; s < strains; s++) {
    char status = get_strain_status(s);
    if (status == 'S' || status == 'R')
      continue;
    if (is_on_av(day-1, s) != -1) {
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

void Health::update_antivirals(int day) {
  for (int s = 0; s < strains; s++) {
    char status = self->get_strain_status(s);
    //We must decide whether someone is going to take the AV
    // As a first cut, we will only give it to a percentage of sympt
    // This will emulate hospitalization
    if(AVs->do_av() && AVs->get_total_current_stock() > 0){
      if(AVs->get_prophylaxis_start_date() == -1 && status == 'I' && day == infection[s]->get_symptomatic_date()){
	// Roll to see if they get one (only do once!!!)
	// This will be replaced by hospitilazation
	
	double r = RANDOM()*100.;
	if(r < AVs->get_percent_symptomatics_given()){
	  int av_to_give = AVs->give_which_AV(s);
	  if(av_to_give != -1){
	    Antiviral* av =  AVs->get_AV(av_to_give);
	    int day_to_give = day + av->roll_start_day();
	    if(Verbose > 0) {
	      cout << "Person " << self->get_id() << " Taking AV "<< av_to_give 
		   << " on day "<< day_to_give << "\n";
	    }
	    take(av, day_to_give);
	  }
	}
      } else if (AVs->get_prophylaxis_start_date() == day) {
	double r = RANDOM()*100.;
	if(r < AVs->get_percent_symptomatics_given()){
	  int av_to_give = AVs->give_which_AV(s);
	  if(av_to_give != -1){
	    Antiviral* av =  AVs->get_AV(av_to_give);
	    if(Verbose > 0) {
	      cout << "Person " << self->get_id() << " Taking AV "<< av_to_give 
		   << " on day "<< day << "\n";
	    }
	    take(av, day);
	  }
	}
      }
      if(Verbose > 3)
	cout << "Number of AVs left = " << AVs->get_total_current_stock() 
	     << " " << AVs->get_AV(0)->get_stock() << "\n";
      
      int iav = is_on_av(day,s);
      if(iav != -1){
	if(day == antiviral_start_date[iav]){
	  if(Verbose > 0) {
	    cout << "AV altering person " << self->get_id() << " down from " 
		 << self->get_susceptibility(s) << " " << self->get_infectivity(s) 
		 << " " << self->get_recovered_date(s) << "\n";
	    printf("susc: %.2f\n", self->get_susceptibility(s));
	  }
	  modify_susceptibility(s,avs[iav]->get_reduce_susceptibility());
	  modify_infectivity(s,avs[iav]->get_reduce_infectivity());
	  modify_develops_symptoms(s,avs[iav]->roll_will_have_symp(), day);
	  modify_asymptomatic_period(s,avs[iav]->get_reduce_asymp_period(), day);
	  modify_symptomatic_period(s,avs[iav]->get_reduce_symp_period(), day);
	  if(Verbose > 0) {
	    cout << " to " << self->get_susceptibility(s) << " " << self->get_infectivity(s) 
		 << " " << self->get_recovered_date(s) << "\n";
	    printf("susc: %.2f\n", self->get_susceptibility(s));
	  }
	}
	else if(day == antiviral_start_date[iav]+avs[iav]->get_course_length()-1){
	  if(Verbose > 1)
	    cout << "AV altering "<<self->get_id() << " up \n";
	  modify_susceptibility(s,1.0/avs[iav]->get_reduce_susceptibility());
	  modify_infectivity(s,1.0/avs[iav]->get_reduce_infectivity());
	  modify_asymptomatic_period(s,1.0/avs[iav]->get_reduce_asymp_period(), day);
	  modify_symptomatic_period(s,1.0/avs[iav]->get_reduce_symp_period(), day);
	}
      }
    }
  }
}

void Health::become_exposed(Infection * infection_ptr) {
  Strain * strain = infection_ptr->get_strain();
  int strain_id = strain->get_id();
  if (Verbose > 0) {
    if (infection_ptr->get_infected_place_id() == -1)
      fprintf(Statusfp, "EXPOSED_DUMMY person %d to strain %d\n", self->get_id(), strain_id);
    else
      fprintf(Statusfp, "EXPOSED person %d to strain %d\n", self->get_id(), strain_id);
  }
  if (infection[strain_id] != NULL) {
    if (infection[strain_id]->get_strain_status() != 'R') {
      printf("WARNING! Attempted double exposure for strain %i in person %i. "
	     "Double infection is not supported.\n",
	     strain_id, self->get_id());
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
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", self->get_id(), strain_id);
    fflush(Statusfp);
  }
  infection[strain_id]->become_infectious();
  strain->remove_from_exposed_list(self);
  strain->insert_into_infectious_list(self);
  if (Verbose > 1) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d has status %c\n",
	    self->get_id(), strain_id, get_strain_status(strain_id));
    fflush(Statusfp);
  }
}

void Health::recover(Strain * strain) {
  int strain_id = strain->get_id();
  infection[strain_id]->recover();
  strain->remove_from_infectious_list(self);
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
void Health::modify_susceptibility(int strain, double multp){
  susceptibility_multp[strain] *= multp;
}

void Health::modify_infectivity(int strain, double multp){
  if (infection[strain] != NULL)
    infection[strain]->modify_infectivity(multp);
}

void Health::modify_infectious_period(int strain, double multp, int cur_day){
  if (infection[strain] != NULL)
    infection[strain]->modify_infectious_period(multp, cur_day);
}

void Health::modify_asymptomatic_period(int strain, double multp, int cur_day){
  if (infection[strain] != NULL)
    infection[strain]->modify_asymptomatic_period(multp, cur_day);
}

void Health::modify_symptomatic_period(int strain, double multp, int cur_day){
  if (infection[strain] != NULL)
    infection[strain]->modify_symptomatic_period(multp, cur_day);
}

void Health::modify_develops_symptoms(int strain, bool symptoms, int cur_day){
  if (infection[strain] != NULL &&
      (infection[strain]->get_strain_status() == 'i' ||
       infection[strain]->get_strain_status() == 'E'))
    infection[strain]->modify_develops_symptoms(symptoms, cur_day);
}

//Medication operators
int Health::take(Antiviral *av, int day){
  antiviral_start_date.push_back(day);
  avs.push_back(av);
  number_av_taken++;
  // Check for efficacy ( or resistance)
  if(av->roll_efficacy()!=0){
    av->add_ineff_given_out(1);
  }
  av->add_given_out(1);
  av->reduce_stock(1);
  return 1;
}

int Health::is_on_av(int day, int s){
  if(number_av_taken ==0){
    return -1;
  }
  //loop through to see if they are on any avs
  else{
    for(int iav=0; iav<number_av_taken; iav++){
      if(avs[iav]->get_strain() == s){
	if(day >= antiviral_start_date[iav] && 
	   day < antiviral_start_date[iav]+avs[iav]->get_course_length()){
	  return iav;
	}
      }
    }
  }
  return -1;
}

const Antiviral* Health::get_av(int day, int s){
  int index = is_on_av(day, s);
  if (index != -1) {
    return avs[index];
  }
  return NULL;
}

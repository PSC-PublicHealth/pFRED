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
  infection = new vector <Infection *> [strains];
  reset();
}

void Health::reset() {
  number_av_taken=0;
  for (int strain = 0; strain < strains; strain++) {
    for (std::size_t i = 0; i < infection[strain].size(); ++i) {
      delete infection[strain][i];
    }
    infection[strain].clear();
  }
}

void Health::update(int day) {
  //  AVs->print();
  // Possibly mutate each active strain
  // TODO(alona) should consider strains in random order, but that
  // is not relevant right now (only 1 or 2 strains)
  for (int s = 0; s < strains; s++) {
    char status = get_strain_status(s);
    if (status == 'S' || status == 'R')
      continue;
    if (infection[s][0]->possibly_mutate(day)) {
      if (Verbose > 1) {
	fprintf(Statusfp, "INFECTIOUS person %d strain %d mutated on day %i\n",
		self->get_id(), s, day);
	fflush(Statusfp);
      }
      break;
    }
  }

  for (int s = 0; s < strains; s++) {
    char status = get_strain_status(s);
    if (status == 'S')
      continue;
    Strain * strain = infection[s][0]->get_strain();
    if (status == 'E') {
      if (day == get_infectious_date(s)) {
	self->become_infectious(strain);
	status = get_strain_status(s);
      }
    }
    if (status == 'I' || status == 'i') {
      if (day == get_recovered_date(s)) {
	self->recover(strain);
	status = get_strain_status(s);
      }
    }
    //We must decide whether someone is going to take the AV
    // As a first cut, we will only give it to a percentage of sympt
    // This will emulate hospitalization
    if(AVs->do_av() && AVs->get_total_current_stock() > 0){
      if( status == 'I' ){
	if(day == get_infectious_date(s)+1){
	  // Roll to see if they get one (only do once!!!)
	  // This will be replaced by hospitilazation
	  
	  double r = RANDOM()*100.;
	  if(r < AVs->get_percent_symptomatics_given()){
	    int av_to_give = AVs->give_which_AV(s);
	    if(av_to_give != -1){
	      if(Verbose > 0)
		cout << "\nPerson " << self->get_id() << " Taking AV "<< av_to_give 
		     << " on day "<<day;
	      take(AVs->get_AV(av_to_give),day);
	    }
	  }
	}
	if(Verbose > 0)
	  cout << "\nNumber of AVs left = " << AVs->get_total_current_stock() 
	       << " " << AVs->get_AV(0)->get_stock();
      }
      
      int iav = is_on_av(day,s);
      if(iav != -1){
	if(day == antiviral_start_date[iav]){
	  if(Verbose > 0)
	    cout << "\nAV altering "<<self->get_id() << " down from " 
		 << self->get_susceptibility(s) << " " << self->get_infectivity(s) 
		 << " " << self->get_recovered_date(s);
	  modify_susceptibility(s,avs[iav]->get_reduce_susceptibility());
	  modify_infectivity(s,avs[iav]->get_reduce_infectivity());
	  modify_infectious_period(s,avs[iav]->get_reduce_infectious_period());
	  cout << " to " << self->get_susceptibility(s) << " " << self->get_infectivity(s) 
	       << " " << self->get_recovered_date(s);
	  
	}
	else if(day == antiviral_start_date[iav]+avs[iav]->get_course_length()-1){
	  if(Verbose > 1)
	    cout << "\nAV altering "<<self->get_id() << " up";
	  modify_susceptibility(s,1.0/avs[iav]->get_reduce_susceptibility());
	  modify_infectivity(s,1.0/avs[iav]->get_reduce_infectivity());
	  modify_infectious_period(s,1.0/avs[iav]->get_reduce_infectious_period());
	}
      }
    }
  }
}
void Health::become_exposed(Infection * infection_ptr) {
  Strain * strain = infection_ptr->get_strain();
  int strain_id = strain->get_id();
  if (Verbose > 1) {
    fprintf(Statusfp, "EXPOSED person %d to strain %d\n", self->get_id(), strain_id);
  }
  if (!infection[strain_id].empty()) {
    if (infection[strain_id][0]->get_strain_status() != 'R') {
      printf("WARNING! Attempted double exposure for strain %i in person %i. "
	     "Double infection is not supported.\n",
	     strain_id, self->get_id());
      delete infection_ptr;
      return;
    }
    delete infection[strain_id][0];
    infection[strain_id][0] = infection_ptr;
  } else {
    infection[strain_id].push_back(infection_ptr);
  }
  strain->insert_into_exposed_list(self);

  if (strains > 1) {
    // HACK - this is probably NOT how we want to do this.  But strain
    // representation/mutation is still a very open problem for us.
    // become immune to equivalent strains.
    for (int i = 0; i < strains; i++) {
      if (!infection[i].empty())
	continue;
      // Could also check some sort of antigenic status here, but for now,
      // just assume all strains are antigenically identical.
      Strain* s = Pop.get_strain(i);
      Infection* dummy_i =
	Infection::get_dummy_infection(s, self, infection_ptr->get_exposure_date());
      infection[i].push_back(dummy_i);
      self->recover(s);
    }
  }
}

void Health::become_infectious(Strain * strain) {
  int strain_id = strain->get_id();
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", self->get_id(), strain_id);
    fflush(Statusfp);
  }
  infection[strain_id][0]->become_infectious();
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
  infection[strain_id][0]->recover();
  strain->remove_from_infectious_list(self);
}

int Health::is_symptomatic() {
  for (int strain = 0; strain < strains; strain++) {
    if (!infection[strain].empty() && infection[strain][0]->is_symptomatic())
      return 1;
  }
  return 0;
}

int Health::get_exposure_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_exposure_date();
}

int Health::get_infectious_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infectious_date();
}

int Health::get_recovered_date(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_recovered_date();
}

int Health::get_infector(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infector();
}

int Health::get_infected_place(int strain) {
  if (infection[strain].empty()) 
    return -1;
  else
    return infection[strain][0]->get_infected_place_id();
}

char Health::get_infected_place_type(int strain) {
  if (infection[strain].empty()) 
    return 'X';
  else
    return infection[strain][0]->get_infected_place_type();
}

int Health::get_infectees(int strain) {
  if (infection[strain].empty()) 
    return 0;
  else
    return infection[strain][0]->get_infectees();
}

int Health::add_infectee(int strain) {
  if (infection[strain].empty())
    return 0;
  else
    return infection[strain][0]->add_infectee();
}

double Health::get_susceptibility(int strain) {
  if (infection[strain].empty())
    return 1.0;
  else
    return infection[strain][0]->get_susceptibility();
}

double Health::get_infectivity(int strain) {
  if (infection[strain].empty())
    return 0.0;
  else
    return infection[strain][0]->get_infectivity();
}


//Modify Operators
void Health::modify_susceptibility(int strain, double multp){
  infection[strain][0]->modify_susceptibility(multp);
}

void Health::modify_infectivity(int strain, double multp){
  infection[strain][0]->modify_infectivity(multp);
}

void Health::modify_infectious_period(int strain, double multp){
  infection[strain][0]->modify_infectious_period(multp);
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
	if(day < antiviral_start_date[iav]+avs[iav]->get_course_length()){
	  return iav;
	}
      }
    }
  }
  return -1;
}
 
//int replace(Antiviral *av, int nav, int day){
//  if(number_av_taken==
  
    
  

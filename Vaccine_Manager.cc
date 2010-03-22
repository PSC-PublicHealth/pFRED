/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: AV_Manager.cpp
//

#include "Manager.h"
#include "Vaccine_Manager.h"
#include "Policy.h"
#include "Vaccine_Priority_Policies.h"
#include "Population.h"
#include "Vaccines.h"
#include "Person.h"
#include "Health.h"
#include "Behavior.h"
#include "Params.h"
#include "Random.h"

Vaccine_Manager::Vaccine_Manager(Population *P):
  Manager(P){
  Pop = P;
  
  char vaccfile[255];
  get_param((char*)"vaccine_file",vaccfile);
  //if(strcmp(vaccfile,"none")!=0){
  string vfile(vaccfile);
  
  if(vfile.substr(0,4)!="none"){
    get_param((char*)"vaccine_compliance",&vaccine_compliance); // to be put in vaccine_manager
    Vaccs = new Vaccines(vfile.substr(0,vfile.size()-1));  
    Vaccs->print();
    do_vacc = 1;
  }
  else{    // No vaccination specified.
    vaccine_priority_scheme = -1;
    vaccine_priority_age_low = -1;
    vaccine_priority_age_high = -1;
    vaccination_capacity = -1;
    do_vacc = 0;
    return;
  }
  // Priority Age
  int do_age_priority;
  vaccine_priority_scheme = VACC_NO_PRIORITY;
  get_param((char*)"vaccine_prioritize_by_age",&do_age_priority);
  if(do_age_priority){
    
    cout <<"\n Vaccination Priority by Age";
    vaccine_priority_scheme = VACC_AGE_PRIORITY;
    get_param((char*)"vaccine_priority_age_low",&vaccine_priority_age_low);
    get_param((char*)"vaccine_priority_age_high",&vaccine_priority_age_high);
    cout <<"\n      Between Ages "<< vaccine_priority_age_low << " and " << vaccine_priority_age_high;
    cout <<"\n";
  }
  else{
    vaccine_priority_age_low = 0;
    vaccine_priority_age_high = 110;
  }

  get_param((char*)"vaccination_capacity",&vaccination_capacity);

  // Need to fill the AV_Manager Policies
  Queue_Policies.push_back(new Vaccine_Priority_Policy_No_Priority(this)); 
  Queue_Policies.push_back(new Vaccine_Priority_Policy_Specific_Age(this));
  
  current_priority_queue_position = 0;
  current_reg_queue_position = 0;
  
  current_policy = vaccine_priority_scheme;
  
  //Fill_Queues();
};

void Vaccine_Manager::Fill_Queues(void){
  
  if(!do_vacc) return;
  // We need to loop over the entire population that the Manager oversees to put them in a queue.
  int popsize = Pop->get_pop_size();
  Person* people = Pop->get_pop();
  
  for(int ip = 0; ip < popsize; ip++){
    current_person = &people[ip];
    if(Queue_Policies[current_policy]->choose()==1){
      Priority_Queue.push_back(current_person);
    }
    else{
      Queue.push_back(current_person);
    }
  }
  
  //FYShuffle<Person *>(Queue);
  //FYShuffle<Person *>(Priority_Queue);  
  
  cout << "\n Vaccine Queue Stats ";
  cout << "\n   Number in Priority Queue      = "<< Priority_Queue.size();
  cout << "\n   Number in Regular Queue       = "<< Queue.size();
  cout << "\n   Total Agents in Vaccine Queue = "<< Priority_Queue.size() +  Queue.size();
}

void Vaccine_Manager::update(int day){
  if(do_vacc == 1){
    current_day = day;
    Vaccs->update(day);
    cout << "\nCurrent Vaccine Stock = " << Vaccs->get_vaccine(0)->get_current_stock();
    Vaccinate();
  }
}

void Vaccine_Manager::reset(void){
  current_day = -1;
  current_person = NULL;
  current_strain = -1;
  Priority_Queue.clear();
  Queue.clear();
  if(do_vacc){
    Fill_Queues();
    Vaccs->reset();
  }
}

void Vaccine_Manager::print(void){
  Vaccs->print();
}

int Vaccine_Manager::Vaccinate(void){
  
  if(!do_vacc) return 0;
  
  int number_vaccinated = 0;
  int n_p_vaccinated = 0;
  int n_r_vaccinated = 0;
  //  int p_iter_flag = 0;
  //  int q_iter_flag = 0;
  int total_vaccines_avail = Vaccs->get_total_vaccines_avail_today();
  
  if(total_vaccines_avail == 0){ 
    cout <<"\nNo Vaccine Available on Day "<<current_day;
    return 0;
  }

  // Start vaccinating Priority
  
  vector < Person* >:: iterator ip;
  ip = Priority_Queue.begin();
  
  // Run through the priority Queue first 
  while(ip!=Priority_Queue.end()){
    current_person = *ip;
    
    int vacc_app = Vaccs->pick_from_applicable_vaccines(current_person->get_age());
    if(vacc_app > -1){
      if(current_person->get_behavior()->compliance_to_vaccination()){
	number_vaccinated++;
	n_p_vaccinated++;
	Vaccine* vacc = Vaccs->get_vaccine(vacc_app);
	vacc->remove_stock(1);
	total_vaccines_avail--;
	current_person->get_health()->take(vacc,current_day);
      }
      ip = Priority_Queue.erase(ip);  // remove a vaccinated person from the queue
    }
    else{
      ip++;
    }

    if(total_vaccines_avail == 0 || number_vaccinated >= vaccination_capacity){
      cout << "\nVaccinated priority to capacity "<< n_p_vaccinated << " agents, for a total of "<< number_vaccinated << " on day " << current_day;
      cout << "\nLeft in Queues:  Priority ("<< Priority_Queue.size() << ")    Regular ("<<Queue.size() << ")";
      cout << "\n";
      return number_vaccinated;
    }
  }

  cout << "\nVaccinated priority to population " << n_p_vaccinated << " agents, for a total of "<< number_vaccinated << " on day " << current_day;
    
    // Run now through the regular queue
  ip = Queue.begin();
  
  while(ip!=Queue.end()){
    current_person = *ip;
    
    int vacc_app = Vaccs->pick_from_applicable_vaccines(current_person->get_age());
    if(vacc_app > -1){
      if(current_person->get_behavior()->compliance_to_vaccination()){
	number_vaccinated++;
	n_r_vaccinated++;
	Vaccine* vacc = Vaccs->get_vaccine(vacc_app);
	vacc->remove_stock(1);
	total_vaccines_avail--;
	current_person->get_health()->take(vacc,current_day);
      }
      ip = Queue.erase(ip);  // remove a vaccinated person from the queue whether or not they are compliant.
    }
    else{
      ip++;
    }
    
    if(total_vaccines_avail == 0 || number_vaccinated >= vaccination_capacity){
      cout << "\nVaccinated regular to capacity "<< n_r_vaccinated << " agents, for a total of "<< number_vaccinated << " on day " << current_day;
      cout << "\nLeft in Queues:  Priority ("<< Priority_Queue.size() << ")    Regular ("<<Queue.size() << ")";
      cout << "\n";
      return number_vaccinated;
    }
  }
  cout << "\nVaccinated regular to population " << n_r_vaccinated << " agents, for a total of "<< number_vaccinated << " on day " << current_day;
  cout << "\nLeft in Queues:  Priority ("<< Priority_Queue.size() << ")    Regular ("<<Queue.size() << ")";
  cout << "\n";
  return number_vaccinated;
}

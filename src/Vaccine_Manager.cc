/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Vaccine_Manager.cpp
//

#include "Manager.h"
#include "Vaccine_Manager.h"
#include "Policy.h"
#include "Vaccine_Priority_Policies.h"
#include "Population.h"
#include "Vaccines.h"
#include "Vaccine.h"
#include "Person.h"
#include "Health.h"
#include "Behavior.h"
#include "Params.h"
#include "Random.h"
#include "Global.h"
#include "Timestep_Map.h"

Vaccine_Manager::Vaccine_Manager(){
  vaccine_package = NULL;
  vaccine_priority_scheme = -1;
  vaccine_priority_age_low = -1;
  vaccine_priority_age_high = -1;
  current_vaccine_capacity = -1;
  vaccination_capacity_map = NULL;
  do_vacc = false;
}

Vaccine_Manager::Vaccine_Manager(Population *_pop):
Manager(_pop) {
  
  pop = _pop;
  
  vaccine_package = new Vaccines();
  int num_vaccs = 0;
  get_param_from_string("number_of_vaccines",&num_vaccs);
  if(num_vaccs > 0){
    vaccine_package->setup();
    vaccine_package->print();
    do_vacc = 1;
  }
  else{    // No vaccination specified.
    vaccine_priority_scheme = -1;
    vaccine_priority_age_low = -1;
    vaccine_priority_age_high = -1;
    vaccination_capacity_map = NULL;
    current_vaccine_capacity = -1;
    vaccine_dose_priority = -1;
    do_vacc = false;
    return;
  }
  // Priority Age
  int do_age_priority;
  current_policy = VACC_NO_PRIORITY;
  get_param((char*)"vaccine_prioritize_by_age",&do_age_priority);
  if(do_age_priority){ 
    cout <<"Vaccination Priority by Age\n";
    current_policy = VACC_AGE_PRIORITY;
    get_param((char*)"vaccine_priority_age_low",&vaccine_priority_age_low);
    get_param((char*)"vaccine_priority_age_high",&vaccine_priority_age_high);
    cout <<"      Between Ages "<< vaccine_priority_age_low << " and " 
    << vaccine_priority_age_high << "\n";
  }
  else{
    vaccine_priority_age_low = 0;
    vaccine_priority_age_high = 110;
  }
  
  // get vaccine_dose_priority
  get_param((char*)"vaccine_dose_priority",&vaccine_dose_priority);
  assert(vaccine_dose_priority < 4);
  //get_param((char*)"vaccination_capacity",&vaccination_capacity);
  vaccination_capacity_map = new Timestep_Map("vaccination_capacity");
  if( Verbose > 1 )
    vaccination_capacity_map->print();
  
  // Need to fill the Vaccine_Manager Policies
  policies.push_back(new Vaccine_Priority_Policy_No_Priority(this)); 
  policies.push_back(new Vaccine_Priority_Policy_Specific_Age(this));
  
};

Vaccine_Manager::~Vaccine_Manager(){
  if(vaccine_package != NULL) delete vaccine_package;
  if(vaccination_capacity_map != NULL) delete vaccination_capacity_map;
}

void Vaccine_Manager::fill_queues(){
  
  if (!do_vacc) return;
  // We need to loop over the entire population that the Manager oversees to put them in a queue.
  int popsize = pop->get_pop_size();
  Person** people = pop->get_pop();
  
  for (int ip = 0; ip < popsize; ip++){
    Person* current_person = people[ip];
    if (policies[current_policy]->choose(current_person, 0, 0) == 1)
      priority_queue.push_back(current_person);
    else
      queue.push_back(current_person);
  }
  
  vector <Person *> random_queue(queue.size());
  copy(queue.begin(),queue.end(),random_queue.begin());
  FYShuffle < Person *>(random_queue);
  copy(random_queue.begin(), random_queue.end(),queue.begin());
  
  vector <Person *> random_priority_queue(priority_queue.size());			
  copy(priority_queue.begin(),priority_queue.end(),random_priority_queue.begin());			
	FYShuffle <Person *> (random_priority_queue); 
	copy(random_priority_queue.begin(),random_priority_queue.end(),priority_queue.begin());
  
  if (Verbose > 0) {
    cout << "Vaccine Queue Stats \n";
    cout << "   Number in Priority Queue      = " << priority_queue.size() << "\n";
    cout << "   Number in Regular Queue       = " << queue.size() << "\n";
    cout << "   Total Agents in Vaccine Queue = " << priority_queue.size() + queue.size() << "\n";
  }
}

void Vaccine_Manager::add_to_priority_queue_random(Person* person){
  // Find a position to put the person in
  int size = priority_queue.size();
  int position = (int)(RANDOM()*size);
  
  list<Person*>::iterator pq = priority_queue.begin();
  for(int i = 0; i < position; i++) ++pq;
  priority_queue.insert(pq,person);
}

void Vaccine_Manager::add_to_regular_queue_random(Person* person){
  // Find a position to put the person in
  int size = queue.size();
  int position = (int)(RANDOM()*size);
  
  list<Person*>::iterator pq = queue.begin();
  for(int i = 0; i < position; i++) ++pq;
  queue.insert(pq,person);
}

void Vaccine_Manager::add_to_priority_queue_begin(Person* person){
  priority_queue.push_front(person);
}

void Vaccine_Manager::add_to_priority_queue_end(Person* person){
  priority_queue.push_back(person);
}

string Vaccine_Manager::get_vaccine_dose_priority_string() const {
  switch(vaccine_dose_priority){
  case VACC_DOSE_NO_PRIORITY:
    return "No Priority";
  case VACC_DOSE_FIRST_PRIORITY:
    return "Priority, Place at Beginning of Queue";
  case VACC_DOSE_RAND_PRIORITY:
    return "Priority, Place with other Priority";
  case VACC_DOSE_LAST_PRIORITY:
    return "Priority, Place at End of Queue";
  default:
    return "WARNING!!! Unrecognized Vaccine Dose Priority";
  }
  return "WARNING!!! Unrecognized Vaccine Dose Priority";
}
  
    
void Vaccine_Manager::update(int day){
  if (do_vacc == 1) {
    vaccine_package->update(day);
    // Update the current vaccination capacity
    current_vaccine_capacity = vaccination_capacity_map->get_value_for_timestep(day);
    cout << "Current Vaccine Stock = " << vaccine_package->get_vaccine(0)->get_current_stock()  << "\n";
    vaccinate(day);
  }
}

void Vaccine_Manager::reset(){
  priority_queue.clear();
  queue.clear();
  if(do_vacc){
    fill_queues();
    vaccine_package->reset();
  }
}

void Vaccine_Manager::print(){
  vaccine_package->print();
}

void Vaccine_Manager::vaccinate(int day) {
  if (do_vacc)
    cout << "Vaccinating!\n";
  else {
    cout << "Not vaccinating!\n";
    return;
  }
  
  int number_vaccinated = 0;
  int n_p_vaccinated = 0;
  int n_r_vaccinated = 0;
  // Figure out the total number of vaccines we can hand out today
  int total_vaccines_avail = vaccine_package->get_total_vaccines_avail_today();
  
  if(Debug > 1) {
    cout << "Vaccine Capacity on Day "<<day << " = " << current_vaccine_capacity << "\n";
    cout << "Queues at beginning of vaccination:  priority ("<< priority_queue.size() << ")    Regular ("
        <<queue.size() << ")\n";
  }
  if(total_vaccines_avail == 0 || current_vaccine_capacity == 0) {
    if(Debug > 1){
      cout <<"No Vaccine Available on Day "<< day << "\n";
    }
    return;
  }
  
  // Start vaccinating Priority
  list < Person* >:: iterator ip;
  ip = priority_queue.begin();
  
  // Run through the priority queue first 
  while(ip!=priority_queue.end()) {
    Person* current_person = *ip;
    
    int vacc_app = vaccine_package->pick_from_applicable_vaccines(current_person->get_age());
    if(vacc_app > -1){
      bool accept_vaccine = false;
      if(current_person->get_health()->is_vaccinated()) {
	accept_vaccine = current_person->get_behavior()->acceptance_of_another_vaccine_dose();
      }
      else {
	accept_vaccine = current_person->get_behavior()->acceptance_of_vaccine();
      }
      if(accept_vaccine==true){
	number_vaccinated++;
	current_vaccine_capacity--;
	n_p_vaccinated++;
	Vaccine* vacc = vaccine_package->get_vaccine(vacc_app);
	vacc->remove_stock(1);
	total_vaccines_avail--;
	current_person->get_health()->take(vacc,day,this);
	ip = priority_queue.erase(ip);  // remove a vaccinated person 
      }
      else {
	// skip non-compliant person under HBM
	if(strcmp(Cognitive_model_type,"HBM") == 0) ++ip;
	// remove non-compliant person if not HBM
	else ip = priority_queue.erase(ip);
      }
    }
    else {
      if(Debug > 1) {
	cout << "Vaccine not applicable for agent "<<current_person->get_id() << " " \
	     << current_person->get_age() << "\n";
      }
      ++ip;
    }
    
    if(total_vaccines_avail == 0) {
      if(Debug > 1) {
        cout << "Vaccinated priority to stock out "<< n_p_vaccinated << " agents, for a total of "
        << number_vaccinated << " on day " << day << "\n";
        cout << "Left in queues:  Priority ("<< priority_queue.size() << ")    Regular ("
        <<queue.size() << ")\n";
      }
      return;
    }
    if(current_vaccine_capacity == 0) {
      if(Debug > 1) {
        cout << "Vaccinated priority to capacity "<< n_p_vaccinated << " agents, for a total of "
        << number_vaccinated << " on day " << day << "\n";
        cout << "Left in queues:  Priority ("<< priority_queue.size() << ")    Regular ("
        <<queue.size() << ")\n";
      }
      return;
    }
  }
  
  if(Verbose > 1) 
    cout << "Vaccinated priority to population " << n_p_vaccinated 
    << " agents, for a total of "<< number_vaccinated << " on day " 
    << day << "\n";
  
  // Run now through the regular queue
  ip = queue.begin();
  
  while(ip != queue.end()){
    Person* current_person = *ip;
    
    int vacc_app = vaccine_package->pick_from_applicable_vaccines(current_person->get_age());
    if(vacc_app > -1){
      bool accept_vaccine = true;
      if(current_person->get_health()->is_vaccinated()) {
	accept_vaccine = current_person->get_behavior()->acceptance_of_another_vaccine_dose();
      }
      else {
	accept_vaccine = current_person->get_behavior()->acceptance_of_vaccine();
      }
      if(accept_vaccine==true){
        number_vaccinated++;
	current_vaccine_capacity--;
        n_r_vaccinated++;
        Vaccine* vacc = vaccine_package->get_vaccine(vacc_app);
        vacc->remove_stock(1);
        total_vaccines_avail--;
        current_person->get_health()->take(vacc,day,this);
	ip = queue.erase(ip);  // remove a vaccinated person 
      }
      else {
	// skip non-compliant person under HBM
	if(strcmp(Cognitive_model_type,"HBM") == 0) ip++;
	// remove non-compliant person if not HBM
	else ip = queue.erase(ip);
      }
    }
    else{
      ip++;
    }   
    if(total_vaccines_avail == 0) {
      if(Verbose > 0){
        cout << "Vaccinated regular to stock_out "<< n_r_vaccinated << " agents, for a total of "
        << number_vaccinated << " on day " << day << "\n";
        cout << "Left in queues:  priority ("<< priority_queue.size() << ")    Regular ("
        <<queue.size() << ")\n";
      }
      return;
    }
    if(current_vaccine_capacity == 0){
      if(Verbose > 0){
        cout << "Vaccinated regular to capacity "<< n_r_vaccinated << " agents, for a total of "
        << number_vaccinated << " on day " << day << "\n";
        cout << "Left in queues:  priority ("<< priority_queue.size() << ")    Regular ("
        <<queue.size() << ")\n";
      }
      return;
    }
  }
  if(Verbose > 0){
    cout << "Vaccinated regular to population " << n_r_vaccinated 
    << " agents, for a total of "<< number_vaccinated << " on day " << day << "\n";
    cout << "Left in queues:  priority ("<< priority_queue.size() << ")    Regular ("
    <<queue.size() << ")\n";
  }
  return;
}

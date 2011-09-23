/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Person.h
//

#ifndef _FRED_PERSON_H
#define _FRED_PERSON_H

#include "Global.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
using namespace std;

class Population;
class Place;
class Disease;
class Infection;
class Population;
class Transmission;
#include "Demographics.h"
#include "Health.h"
#include "Behavior.h"
#include "Activities.h"


class Person {
public:

  /**
   * Default constructor
   */
  Person();

  /**
   * Constructor that sets all of the attributes of a Person object
   * @param index the person id
   * @param age
   * @param sex (M or F)
   * @param marital 1 if married, 0 if not
   * @param occ the code for the Person's occupation
   * @param house pointer to this Person's Household
   * @param school pointer to this Person's School
   * @param work pointer to this Person's Workplace
   * @param day the simulation day
   * @param today_is_birthday true if this is a newborn
   */
  Person(int index, int age, char sex, int marital, int occ, Place *house,
	 Place *school, Place *work, int day, bool today_is_birthday);

  ~Person();

  /**
   * Make this agent unsusceptible to the given disease
   * @param disease the disease to reference
   */
  void become_unsusceptible(Disease *disease) {
    health->become_unsusceptible(disease);
  }

  /**
   * Make this agent immune to the given disease
   * @param disease the disease to reference
   */
  void become_immune(Disease *disease);

  /**
   * Print out information about this object with regards to a disease to a file.
   *
   * @param fp a pointer to the file where this print should write
   * @param disease the disease about which to get information
   */
  void print(FILE *fp, int disease) const;

  /**
   * @return <code>true</code> if this agent is symptomatic, <code>false</code> otherwise
   * @see Health::is_symptomatic()
   */
  int is_symptomatic() { return health->is_symptomatic(); }

  /**
   * @param dis the disease to check
   * @return <code>true</code> if this agent is susceptible to disease, <code>false</code> otherwise
   * @see Health::is_susceptible(int dis)
   */
  bool is_susceptible(int dis) { return health->is_susceptible(dis); }

  /**
   * @param dis the disease to check
   * @return <code>true</code> if this agent is infectious with disease, <code>false</code> otherwise
   * @see Health::is_infectious(int disease)
   */
  bool is_infectious(int dis) { return health->is_infectious(dis); }

  /**
   * @return the a pointer to this agent's Household
   */
  Place * get_household() const;

  /**
   * @return the a pointer to this agent's Neighborhood
   */
  Place * get_neighborhood() const;

  /**
   * @return the Person's age
   * @see Demographics::get_age()
   */
  int get_age() const { return demographics->get_age(); }

  /**
   * @return the Person's initial age
   * @see Demographics::get_init_age()
   */
  int get_init_age() const { return demographics->get_init_age(); }

  /**
   * @return the Person's initial profession
   * @see Demographics::get_init_profession()
   */
  int get_init_profession() const { return demographics->get_init_profession(); }

  /**
   * @return the Person's initial marital status
   * @see Demographics::get_init_marital_status()
   */
  int get_init_marital_status() const { return demographics->get_init_marital_status(); }

  /**
   * @return the Person's age as a double value based on the number of days alive
   * @see Demographics::get_real_age()
   */
  double get_real_age(int day) const { return demographics->get_real_age(day); }

  /**
   * @return the Person's sex
   */
  char get_sex() const;

  /**
   * @return the Person's marital status
   */
  int get_marital_status() const;

  /**
   * @return the Person's profession
   */
  int get_profession() const;

  /**
   * @param disease the disease to check
   * @return the specific Disease's status for this Person
   * @see Health::get_disease_status(int disease)
   */
  char get_disease_status(int disease) const { return health->get_disease_status(disease); }

  /**
   * @param disease the disease to check
   * @return the specific Disease's susceptibility for this Person
   * @see Health::get_susceptibility(int disease)
   */
  double get_susceptibility(int disease) const { return health->get_susceptibility(disease); }

  /**
   * @param disease the disease to check
   * @return the specific Disease's infectivity for this Person
   * @see Health::get_susceptibility(int disease)
   */
  double get_infectivity(int disease, int day) const { return health->get_infectivity(disease, day); }

  /**
   * @param disease the disease to check
   * @return the simulation day that this agent became exposed to disease
   */
  int get_exposure_date(int disease) const;

  /**
   * @param disease the disease to check
   * @return the simulation day that this agent became infectious with disease
   */
  int get_infectious_date(int disease) const;

  /**
   * @param disease the disease to check
   * @return the simulation day that this agent became recovered from disease
   */
  int get_recovered_date(int disease) const;

  /**
   * @param disease the disease to check
   * @return the id of the Person who infected this agent with disease
   */
  int get_infector(int disease) const;

  /**
   * @param disease the disease to check
   * @return the id of the location where this agent became infected with disease
   */
  int get_infected_place(int disease) const;

  /**
   * @param disease the disease to check
   * @return the label of the location where this agent became infected with disease
   */
  char * get_infected_place_label(int disease) const;

  /**
   * @param disease the disease to check
   * @return the type of the location where this agent became infected with disease
   */
  char get_infected_place_type(int disease) const;

  /**
   * @param disease the disease in question
   * @return the infectees
   * @see Health::get_infectees(int disease)
   */
  int get_infectees(int disease) const;

  /**
   * @param disease the disease in question
   * @return
   * @see Health::add_infectee(int disease)
   */
  int add_infectee(int disease);

  /**
   * @param day the simulation day
   * @param disease the disease to check
   * @return <code>true</code> if the exposure day to the disease = day, <code>false</code> otherwise
   */
  int is_new_case(int day, int disease) const;


  int addInfected(int disease, vector<int> strains);
  
  void infect(Person *infectee, int disease, Transmission *transmission);
  void getInfected(Disease *disease, Transmission *transmission);
  void addIncidence(int disease, vector<int> strains);
  void addPrevalence(int disease, vector<int> strains);
  
  /**
   * Notify the population that this Person has changed
   */
  void set_changed();

  /**
   * @param day the simulation day
   * @see Demographics::update(int day)
   */
  void update_demographics(int day) { demographics->update(day); }

  /**
   * @param day the simulation day
   * @see Health::update(int day)
   */
  void update_health(int day) { health->update(day); }

  /**
   * @param day the simulation day
   * @see Behavior::update(int day)
   */
  void update_behavior(int day) { behavior->update(day); }

  bool is_staying_home(int day) { return behavior->is_staying_home(day); }

  /**
   * @Activities::update_profile()
   */
  void update_activity_profile() { activities->update_profile(); }

  /**
   * @see Activities::withdraw()
   */
  void withdraw_from_activities() { activities->withdraw(); }

  /**
   * @see Activities::update_household_mobility()
   */
  void update_household_mobility() { activities->update_household_mobility(); }

  /**
   * This agent will become susceptible to the disease
   * @param disease the disease
   */
  void become_susceptible(Disease * disease) { health->become_susceptible(disease); }

  /**
   * This agent will become infectious with the disease
   * @param disease a pointer to the Disease
   */
  void become_infectious(Disease * disease) { health->become_infectious(disease); }

  /**
   * This agent will become symptomatic with the disease
   * @param disease a pointer to the Disease
   */
  void become_symptomatic(Disease *disease) { health->become_symptomatic(disease); }

  /**
   * This agent will recover from the disease
   * @param disease a pointer to the Disease
   */
  void recover(Disease * disease) { health->recover(disease); }

  /**
   * @return <code>true</code> if this agent is deceased, <code>false</code> otherwise
   */
  bool is_deceased() { return demographics->is_deceased(); }

  /**
   * @return <code>true</code> if this agent is an adult, <code>false</code> otherwise
   */
  bool is_adult() { return demographics->get_age() >= Global::ADULT_AGE; }
  
  /**
   * This agent creates a new agent
   * @return a pointer to the new Person
   */
  Person * give_birth(int day);

  /**
   * Assign the agent to a Classroom
   * @see Activities::assign_classroom()
   */
  void assign_classroom() { activities->assign_classroom(); }

  /**
   * Assign the agent to an Office
   * @see Activities::assign_office()
   */
  void assign_office() { activities->assign_office(); }

  /**
   *  @return the number of other agents in an agent's neighborhood, school, and workplace.
   *  @see Activities::get_degree()
   */
  int get_degree() { return activities->get_degree(); }

  int get_household_size(){ return activities->get_group_size(HOUSEHOLD_INDEX); }
  int get_neighborhood_size(){ return activities->get_group_size(NEIGHBORHOOD_INDEX); }
  int get_school_size(){ return activities->get_group_size(SCHOOL_INDEX); }
  int get_classroom_size(){ return activities->get_group_size(CLASSROOM_INDEX); }
  int get_workplace_size(){ return activities->get_group_size(WORKPLACE_INDEX); }
  int get_office_size(){ return activities->get_group_size(OFFICE_INDEX); }

  /**
   * Will print out a person in a format similar to that read from population file
   * (with additional run-time values inserted (denoted by *)):<br />
   * (i.e Label *ID* Age Sex Married Occupation Household School *Classroom* Workplace *Office*)
   * @return a string representation of this Person object
   */
  string to_string();

  // access functions:
  /**
   * The id is generated at runtime
   * @return the id of this Person
   */
  int get_id() const { return idx; }

  /**
   * @return a pointer to this Person's Demographics
   */
  Demographics * get_demographics() const { return demographics; }

  /**
   * @return a pointer to this Person's Health
   */
  Health * get_health() const { return health; }

  /**
   * @return a pointer to this Person's Activities
   */
  Activities * get_activities() const { return activities; }

  /**
   * @return a pointer to this Person's Behavior
   */
  Behavior * get_behavior() const { return behavior; }

  /**
   * @return a pointer to this Person's Household
   * @see Activities::get_household()
   */
  Place * get_household() { return activities->get_household(); }

  /**
   * @return a pointer to this Person's Neighborhood
   * @see Activities::get_neighborhood()
   */
  Place * get_neighborhood() { return activities->get_neighborhood(); }

  /**
   * @return a pointer to this Person's School
   * @see Activities::get_school()
   */
  Place * get_school() { return activities->get_school(); }

  /**
   * @return a pointer to this Person's Classroom
   * @see Activities::get_classroom()
   */
  Place * get_classroom() { return activities->get_classroom(); }

  /**
   * @return a pointer to this Person's Workplace
   * @see Activities::get_workplace()
   */
  Place * get_workplace() { return activities->get_workplace(); }

  /**
   * @return a pointer to this Person's Office
   * @see Activities::get_office()
   */
  Place * get_office() { return activities->get_office(); }

  /**
   * Have this Person begin traveling
   * @param visited the Person this agent will visit
   * @see Activities::start_traveling(Person *visited)
   */
  void start_traveling(Person *visited){ activities->start_traveling(visited); }

  /**
   * Have this Person stop traveling
   * @see Activities::stop_traveling()
   */
  void stop_traveling(){ activities->stop_traveling(); }

  /**
   * @return <code>true</code> if the Person is traveling, <code>false</code> if not
   * @see Activities::get_travel_status()
   */
  bool get_travel_status(){ return activities->get_travel_status(); }

private:
  int idx;              // person id
  Demographics *demographics;
  Health *health;
  Activities *activities;
  Behavior *behavior;
};

#endif // _FRED_PERSON_H

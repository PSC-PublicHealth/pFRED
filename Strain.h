/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Strain.h
//

#ifndef _FRED_STRAIN_H
#define _FRED_STRAIN_H

#include <set>
#include <stack>
using namespace std;

class Infection;
class Person;
class Place;
class Population;
class Spread;
class Age_Map;

class Strain {
public:
	Strain();
  virtual ~Strain();
	
	virtual void reset();
	virtual void setup(int s, Population *pop,  double *mut_prob);
	virtual void print();
	virtual void update(int day);
	
	// If the infectee is susceptible, is at the given place on the given day,
	// and if the random roll for transmissibility succeeds
	// the infection is successful and attempt_infection returns true.
	virtual bool attempt_infection(Person* infector, Person* infectee, Place* place, int exposure_date);
	
	// These methods draw from the underlying distributions to randomly determine some aspect of the infection.
	virtual int get_days_latent();
	virtual int get_days_incubating();
	virtual int get_days_asymp();
	virtual int get_days_symp();
	virtual int get_days_recovered();
	virtual int get_symptoms();
	virtual double get_asymp_infectivity() {return asymp_infectivity;}
	virtual double get_symp_infectivity() {return symp_infectivity;}
	
	virtual int get_id() { return id; }
	virtual double get_transmissibility() { return transmissibility; }
	virtual double get_prob_symptomatic() { return prob_symptomatic; }
	virtual double get_attack_rate();
	virtual Age_Map* get_residual_immunity() const { return residual_immunity; }
	virtual Age_Map* get_at_risk() const { return at_risk; }
	
	// Draws from this strain's mutation probability distribution, and returns
	// the strain to mutate to, or NULL if no mutation should occur.
	Strain* should_mutate_to();
	
	virtual void insert_into_exposed_list(Person *person);
	virtual void insert_into_infectious_list(Person *person);
	virtual void remove_from_exposed_list(Person *person);
	virtual void remove_from_infectious_list(Person *person);
	virtual void update_stats(int day);
	virtual void print_stats(int day);
	
	virtual Population * get_population() { return population; }
	
	static double get_prob_stay_home();
	static void set_prob_stay_home(double);
	static void get_strain_parameters();
private:
	int id;
	double transmissibility;
	double prob_symptomatic;
	double asymp_infectivity;
	double symp_infectivity;
	int max_days_latent;
	int max_days_incubating;
	int max_days_asymp;
	int max_days_symp;
	double *days_latent;
	double *days_incubating;
	double *days_asymp;
	double *days_symp;
	double immunity_loss_rate;
	double *mutation_prob;
	Spread *spread;
	Age_Map *residual_immunity;
  Age_Map *at_risk;
  
	// Vars that are not strain-specific (for updating global stats).
	Population *population;
};


#endif // _FRED_STRAIN_H

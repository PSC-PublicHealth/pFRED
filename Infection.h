/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
// File: Infection.h
//

#ifndef _FRED_INFECTION_H
#define _FRED_INFECTION_H

class Health;
class Person;
class Place;
class Strain;
class Antiviral;
class Health;

extern int SEiIR_model;

class Infection {
public:
	// if primary infection, infector and place are null.
	// if mutation, place is null.
	Infection(Strain *s, Person *infector, Person *infectee, Place* place, int day);
	virtual ~Infection() { }
	
	// infection state
	virtual char get_strain_status() const { return status; }
	virtual void become_infectious();
	virtual void become_symptomatic();
	virtual void become_susceptible();
	virtual void recover();
	virtual void update(int today);
	virtual bool possibly_mutate(Health *health, int day); 	// may cause mutation and/or alter infection course
  
	// general
	virtual Strain *get_strain() const { return strain; }
	virtual Person *get_infector() const { return infector; }
	virtual Place *get_infected_place() const { return place; }
	virtual int get_infectee_count() const { return infectee_count; }
	virtual int add_infectee() { return ++infectee_count; }
	virtual void print() const;
	
	// chrono
	virtual int get_exposure_date() const { return exposure_date; }
	virtual int get_infectious_date() const { return exposure_date + latent_period; }
	virtual int get_symptomatic_date() const { return get_infectious_date() + asymptomatic_period; }
	virtual int get_recovery_date() const { return get_symptomatic_date() + symptomatic_period; }
	virtual int get_susceptible_date() const { return get_recovery_date() + recovery_period; }
	virtual void modify_asymptomatic_period(double multp, int cur_day);
	virtual void modify_symptomatic_period(double multp, int cur_day);
	virtual void modify_infectious_period(double multp, int cur_day);
	
	// parameters
	virtual bool is_symptomatic() const { return symptoms > 0; }
	virtual double get_susceptibility() const { return susceptibility; }
	virtual double get_infectivity() const { return infectivity * infectivity_multp; }
	virtual double get_symptoms() const { return symptoms; }
	virtual void modify_develops_symptoms(bool symptoms, int cur_day);
	virtual void modify_susceptibility(double multp) { susceptibility *= multp; }
	virtual void modify_infectivity(double multp) { infectivity_multp = multp; }	
	
	// returns an infection for the given host and strain with exposed date and
	// recovered date both equal to day (instant resistance to the given strain);
	static Infection* get_dummy_infection(Strain *s, Person *host, int day);
  
private:
	// associated strain
	Strain *strain;
	
	// infection status (E/I/i/R)
	char status;
	
	// chrono data
	int exposure_date;
	int latent_period;
	int asymptomatic_period;
	int symptomatic_period;
	int recovery_period;
  
	// people involved
	Person *infector;
	Person *host;
	
	// where infection was caught
	Place *place;
	
	// number of people infected by this infection
	int infectee_count;
  
	// parameters
	bool will_be_symptomatic;
	double susceptibility;
	double infectivity;
	double infectivity_multp;
	double symptoms;
	
protected:
	// for mocks
	Infection() { }
};

#endif // _FRED_INFECTION_H
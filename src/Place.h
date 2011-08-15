/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Place.h
//

#ifndef _FRED_PLACE_H
#define _FRED_PLACE_H

#define HOUSEHOLD 'H'
#define NEIGHBORHOOD 'N'
#define SCHOOL 'S'
#define CLASSROOM 'C'
#define WORKPLACE 'W'
#define OFFICE 'O'
#define HOSPITAL 'M'
#define COMMUNITY 'X'

#include <vector>
using namespace std;

#include "Population.h"
#include "Global.h"
class Cell;
class Person;


class Place {
public:
  Place() {}
  ~Place() {}

  /**
   *  Sets the id, label, logitude, latitude , container and population of this Place
   *  Allocates disease-related memory for this place
   *
   *  @param loc_id this Place's id
   *  @param lab this Place's label
   *  @param lon this Place's longitude
   *  @param lat this Place's latitude
   *  @param cont this Place's container
   *  @param pop this Place's population
   */
  void setup(int loc_id, const char *lab, double lon, double lat, Place *cont, Population *pop);

  /**
   * Get this place ready
   */
  virtual void prepare();

  /**
   * Perform a daily update of this place.  The daily count arrays will all be reset and the vectors
   * containing infectious and symptomatics will be cleared.
   */
  void update(int day);

  /**
   * Display the information for a given disease.
   *
   * @param disease an integer representation of the disease
   */
  virtual void print(int disease);

  /**
   * Add a person to the place. This method increments the number of people in
   * the place.
   *
   * @param per a pointer to a Person object that may be added to the place
   */
  virtual void enroll(Person * per);

  /**
   * Remove a person from the place. This method decrements the number of people in
   * the place.
   *
   * @param per a pointer to a Person object that may be removed to the place
   */
  virtual void unenroll(Person * per);

  /**
   * Add a susceptible person to the place. This method adds the person to the susceptibles vector and
   * increments the number of susceptibles in the place (S).
   *
   * @param disease an integer representation of the disease
   * @param per a pointer to a Person object that will be added to the place for a given disease
   */
  virtual void add_susceptible(int disease, Person * per);

  /**
   * Add a infectious person to the place. This method adds the person to the infectious vector and
   * increments the number of infectious in the place (I).
   *
   * @param disease an integer representation of the disease
   * @param per a pointer to a Person object that will be added to the place for a given disease
   * @param status a character representation of the status
   */
  virtual void add_infectious(int disease, Person * per, char status);

  /**
   * Prints the id of every person in the susceptible vector for a given disease.
   *
   * @param disease an integer representation of the disease
   */
  void print_susceptibles(int disease);

  /**
   * Prints the id of every person in the infectious vector for a given disease.
   *
   * @param disease an integer representation of the disease
   */
  void print_infectious(int disease);

  /**
   * Attempt to spread the infection for a given disease on a given day.
   *
   * @param day the simulation day
   * @param disease an integer representation of the disease
   */
  virtual void spread_infection(int day, int disease);

  /**
   * Is the place open on a given day?
   *
   * @param day the simulation day
   * @return <code>true</code> if the place is open; <code>false</code> if not
   */
  bool is_open(int day);

  /**
   * Test whether or not any infectious people are in this place.
   *
   * @return <code>true</code> if any infectious people are here; <code>false</code> if not
   */
  bool is_infectious(int disease) { return I[disease] > 0; }

  /**
   * Allow update of incidence vector.
   *
   * @param disease an integer representation of the disease
   * @@param strain a vector of integers representing strains
   * @param incr is unused at this time
   */
  void modifyIncidenceCount(int dis, vector<int> strain, int incr);

  /**
   * Allow update of prevalence vector.
   *
   * @param disease an integer representation of the disease
   * @@param strain a vector of integers representing strains
   * @param incr is unused at this time
   */
  void modifyPrevalenceCount(int dis, vector<int> strain, int incr);

  /**
   * <bold>Deprecated.</bold>
   */
  void print_stats(int day, int disease);
  
  /**
   * Sets the static variables for the class from the parameter file for a given number of diseases.
   *
   * @param disease an integer representation of the disease
   */
  virtual void get_parameters(int disease) = 0;

  /**
   * Get the age group for a person given a particular disease.
   *
   * @param disease an integer representation of the disease
   * @param per a pointer to a Person object
   * @return the age group for the given person for the given disease
   */
  virtual int get_group(int disease, Person * per) = 0;

  /**
   * Get the transmission probability for a given disease between two Person objects.
   *
   * @param disease an integer representation of the disease
   * @param i a pointer to a Person object
   * @param s a pointer to a Person object
   * @return the probability that there will be a transmission of disease from i to s
   */
  virtual double get_transmission_prob(int disease, Person * i, Person * s) = 0;

  /**
   * Get the contacts for a given disease.
   *
   * @param disease an integer representation of the disease
   * @return the contacts per day for the given disease
   */
  virtual double get_contacts_per_day(int disease) = 0; // access functions

  /**
   * Determine if the place should be open. It is dependent on the disease and simulation day.
   *
   * @param day the simulation day
   * @param disease an integer representation of the disease
   * @return <code>true</code> if the place should be open; <code>false</code> if not
   */
  virtual bool should_be_open(int day, int disease) = 0;

  /**
   * Get the id.
   * @return the id
   */
  int get_id() { return id; }

  /**
   * Get the label.
   *
   * @return the label
   */
  char * get_label() { return label; }

  /**
   * Get the type (H)OME, (W)ORK, (S)CHOOL, (C)OMMUNITY).
   *
   * @return the type
   */
  int get_type() { return type; }

  /**
   * Get the latitude.
   *
   * @return the latitude
   */
  double get_latitude() { return latitude; }

  /**
   * Get the longitude.
   *
   * @return the longitude
   */
  double get_longitude() { return longitude; }

  /**
   * Get the count of (S)usceptibles for a given disease in this place.
   *
   * @param disease an integer representation of the disease
   * @return the suceptible count for the given disease
   */
  int get_S(int disease) { return S[disease]; }

  /**
   * Get the count of (I)nfectious for a given disease in this place.
   *
   * @param disease an integer representation of the disease
   * @return the infectious count for the given disease
   */
  int get_I(int disease) { return (int) (infectious[disease].size()); }

  /**
   * Get the count of (S)ymptomatics for a given disease in this place.
   *
   * @param disease an integer representation of the disease
   * @return the symptomatic count for the given disease
   */
  int get_symptomatic(int disease) { return Sympt[disease]; }

  /**
   * Get the count of agents in this place.
   *
   * @return the count of agents
   */
  int get_size() { return N; }

  /**
   * Get the simulation day (an integer value of days from the start of the simulation) when the place will close.
   *
   * @return the close_date
   */
  int get_close_date() { return close_date; }

  /**
   * Get the simulation day (an integer value of days from the start of the simulation) when the place will open.
   *
   * @return the open_date
   */
  int get_open_date() { return open_date; }

  /**
   * Get the population.
   *
   * @return the population
   */
  Population *get_population() { return population; }

  /**
   * Get the number of cases of a given disease for day.
   * The member variable cases gets reset when <code>update()</code> is called, which for now is on a daily basis.
   *
   * @param disease an integer representation of the disease
   * @return the count of cases for a given disease
   */
  int get_daily_cases(int disease) { return cases[disease]; }

  /**
   * Get the number of deaths from a given disease for a day.
   * The member variable deaths gets reset when <code>update()</code> is called, which for now is on a daily basis.
   *
   * @param disease an integer representation of the disease
   * @return the count of deaths for a given disease
   */
  int get_daily_deaths(int disease) { return deaths[disease]; }

  /**
   * Get the number of cases of a given disease for the simulation thus far.
   * This value is not reset when <code>update()</code> is called.
   *
   * @param disease an integer representation of the disease
   * @return the count of cases for a given disease
   */
  int get_total_cases(int disease) { return total_cases[disease]; }

  /**
   * Get the number of deaths from a given disease for the simulation thus far.
   * This value is not reset when <code>update()</code> is called.
   *
   * @param disease an integer representation of the disease
   * @return the count of deaths for a given disease
   */
  int get_total_deaths(int disease) { return total_deaths[disease]; }

  /**
   * Get the number of cases of a given disease for the simulation thus far divided by the
   * number of agents in this place.
   *
   * @param disease an integer representation of the disease
   * @return the count of rate of cases per people for a given disease
   */
  double get_incidence_rate(int disease) { return (double) total_cases[disease] / (double) N; }
  
  /**
   * Set the id.
   *
   * @param n the new id
   */
  void set_id(int n) { id = n; }

  /**
   * Set the type.
   *
   * @param t the new type
   */
  void set_type(char t) { type = t; }

  /**
   * Set the latitude.
   *
   * @param x the new latitude
   */
  void set_latitude(double x) { latitude = x; }

  /**
   * Set the longitude.
   *
   * @param x the new longitude
   */
  void set_longitude(double x) { longitude = x; }

  /**
   * Set the simulation day (an integer value of days from the start of the simulation) when the place will close.
   *
   * @param day the simulation day when the place will close
   */
  void set_close_date(int day) { close_date = day; }

  /**
   * Set the simulation day (an integer value of days from the start of the simulation) when the place will open.
   *
   * @param day the simulation day when the place will open
   */
  void set_open_date(int day) { open_date = day; }

  /**
   * Set the population.
   *
   * @param p the new population
   */
  void set_population(Population *p) {  population = p; }

  /**
   * Set the container.
   *
   * @param cont the new container
   */
  void set_container(Place *cont) { container = cont; }

  /**
   * Increment the cases count. Note that the cases variable will be reset when <code>update()</code> is called
   */
  void add_case() { cases++; }

  /**
   * Increment the deaths count. Note that the deaths variable will be reset when <code>update()</code> is called
   */
  void add_deaths() { deaths++; }

  /**
   * Get the grid_cell where this place is.
   *
   * @return a pointer to the grid_cell where this place is
   */
  Cell * get_grid_cell() { return grid_cell; }

  /**
   * Set the grid_cell where this place will be.
   *
   * @param p the new grid_cell
   */
  void set_grid_cell(Cell *p) { grid_cell = p; }
  
protected:
  int id;					// place id
  char label[32];				// external id
  char type;				// HOME, WORK, SCHOOL, COMMUNITY
  Place *container;				// id of container place
  double latitude;				// geo location
  double longitude;				// geo location
  int N;			   // total number of potential visitors
  vector <Person *> *susceptibles;	 // list of susceptible visitors
  vector <Person *> *infectious;	  // list of infectious visitors
  int *S;					// susceptible count
  int *I;					// infectious count
  int *Sympt;					// symptomatics count
  int close_date;		    // this place will be closed during:
  int open_date;			    //   [close_date, open_date)
  int * cases;					// symptomatic cases today
  int * deaths;					// deaths today
  int * total_cases;			      // total symptomatic cases
  int * total_deaths;				// total deaths
  Population *population;
  int diseases;					// number of diseases
  Cell * grid_cell;			 // geo grid_cell for this place
  vector< map<int, int> > incidence;
  vector< map<int, int> > prevalence;

  // disease parameters
  double *beta;	       // place-independent transmissibility per contact
};

#endif // _FRED_PLACE_H

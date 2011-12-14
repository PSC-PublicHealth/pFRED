/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Workplace.h
//

#ifndef _FRED_WORKPLACE_H
#define _FRED_WORKPLACE_H

#include "Place.h"
#include <vector>



class Workplace: public Place {
public: 

  /**
   * Default constructor
   */
  Workplace() { offices.clear(); next_office = 0; }
  ~Workplace() {}

  /**
   * Convenience constructor that sets most of the values by calling Place::setup
   *
   * @see Place::setup(int loc_id, const char *lab, double lon, double lat, Place* cont, Population *pop)
   */
  Workplace(int, const char *,double,double,Place *, Population *pop);

  /**
   * @see Place::get_parameters(int diseases)
   *
   * This method is called by the constructor
   * <code>Workplace(int loc, const char *lab, double lon, double lat, Place *container, Population* pop)</code>
   */
  void get_parameters(int diseases);

  /**
   * Initialize the workplace and its offices
   */
  void prepare();

  /**
   * @see Place::get_group(int disease, Person * per)
   */
  int get_group(int disease, Person * per) { return 0; }

  /**
    * @see Place::get_transmission_prob(int disease, Person * i, Person * s)
    *
    * This method returns the value from the static array <code>Workplace::Workplace_contact_prob</code> that
    * corresponds to a particular age-related value for each person.<br />
    * The static array <code>Workplace_contact_prob</code> will be filled with values from the parameter
    * file for the key <code>workplace_prob[]</code>.
    */
  double get_transmission_prob(int disease, Person * i, Person * s);

  /**
   * @see Place::get_contacts_per_day(int disease)
   *
   * This method returns the value from the static array <code>Workplace::Workplace_contacts_per_day</code>
   * that corresponds to a particular disease.<br />
   * The static array <code>Workplace_contacts_per_day</code> will be filled with values from the parameter
   * file for the key <code>workplace_contacts[]</code>.
   */
  double get_contacts_per_day(int disease);

  /**
   * Setup the offices within this Workplace
   */
  void setup_offices();

  /**
   * Assign a person to a particular Office
   * @param per the Person to assign
   * @return the Office to which the Person was assigned
   */
  Place * assign_office(Person *per);

  /**
   * Determine if the Workplace should be open. It is dependent on the disease and simulation day.
   *
   * @param day the simulation day
   * @param disease an integer representation of the disease
   * @return whether or not the workplace is open on the given day for the given disease
   */
  bool should_be_open(int day, int disease) { return true; }
  bool is_sick_leave_available() { return sick_leave_available; }
  char get_size_code() { return size_code; }

 private:
  static double * Workplace_contacts_per_day;
  static double *** Workplace_contact_prob;
  static bool Workplace_parameters_set;
  static int Office_size;
  static int Enable_sick_leave;

  vector <Place *> offices;
  int next_office;
  bool sick_leave_available;
  char size_code;
};

#endif // _FRED_WORKPLACE_H


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Activities.h
//

#ifndef _FRED_ACTIVITIES_H
#define _FRED_ACTIVITIES_H

#include <vector>
#include "Global.h"
class Person;
class Place;

#define HOUSEHOLD_INDEX 0
#define NEIGHBORHOOD_INDEX 1
#define SCHOOL_INDEX 2
#define CLASSROOM_INDEX 3
#define WORKPLACE_INDEX 4
#define OFFICE_INDEX 5
#define FAVORITE_PLACES 6
#define MAX_MOBILITY_AGE 100

class Activities {
public:
  Activities (Person *person, Place **favorite_place);
  Activities (Person *person, char *house, char *school, char *work);

  /**
   * Assigns an activity profile to the agent
   */
  void assign_profile();

  /**
   * Perform the daily update for this object.  However, for this object, we avoid computing this except as happens in
   * update_schedule(day)
   *
   * @param day the simulation day
   */
  void update(int day);

  /**
   * Perform the daily update for an infectious agent
   *
   * @param day the simulation day
   */
  void update_infectious_activities(int day);

  /**
   * Perform the daily update for a susceptible agent
   *
   * @param day the simulation day
   */
  void update_susceptible_activities(int day);

  /**
   * Perform the daily update to the schedule
   *
   * @param day the simulation day
   */
  void update_schedule(int day);

  /**
   * Print the Activity schedule
   */
  void print_schedule();

  /**
   * Print out information about this object
   */
  void print();

  /**
   * @return a pointer to this agent's Household
   */
  Place * get_household() { return favorite_place[HOUSEHOLD_INDEX]; }

  /**
   * @return a pointer to this agent's Neighborhood
   */
  Place * get_neighborhood() { return favorite_place[NEIGHBORHOOD_INDEX]; }

  /**
   * @return a pointer to this agent's School
   */
  Place * get_school() { return favorite_place[SCHOOL_INDEX]; }

  /**
   * @return a pointer to this agent's Classroom
   */
  Place * get_classroom() { return favorite_place[CLASSROOM_INDEX]; }

  /**
   * @return a pointer to this agent's Workplace
   */
  Place * get_workplace() { return favorite_place[WORKPLACE_INDEX]; }

  /**
   * @return a pointer to this agent's Office
   */
  Place * get_office() { return favorite_place[OFFICE_INDEX]; }

  /**
   * Assign the agent to a School
   */
  void assign_school();

  /**
   * Assign the agent to a Classroom
   */
  void assign_classroom();

  /**
   * Assign the agent to a Workplace
   */
  void assign_workplace();

  /**
   * Assign the agent to an Office
   */
  void assign_office();

  /**
   * Update the agent's profile
   */
  void update_profile();

  /**
   * Update the household mobility of the agent</br>
   * Note: this is not used unless <code>Global::Enable_Mobility</code> is set to <code>true</code>
   */
  void update_household_mobility();

  /**
   * Unenroll from all the favorite places
   */
  void withdraw();

  /**
   * Increments the incidence count for a given disease and its strains for all of the agent's favorite places
   */
  void addIncidence(int disease, std::vector<int> strains);

  /**
   * Increments the prevalence count for a given disease and its strains for all of the agent's favorite places
   */
  void addPrevalence(int disease, std::vector<int> strains);

  /**
   * The agent begins traveling.  The favorite places for this agent are stored, and it gets a new schedule
   * based on the agent it is visiting.
   *
   * @param visited a pointer to the Person object being visited
   * @see Activities::store_favorite_places()
   */
  void start_traveling(Person *visited);

  /**
   * The agent stops traveling and returns to its original favorite places
   * @see Activities::restore_favorite_places()
   */
  void stop_traveling();

  /**
   * @return <code>true</code> if the agent is traveling, <code>false</code> otherwise
   */
  bool get_travel_status() { return travel_status; }
	
private:
  Person * self;	 // pointer to person using having this activities
  int profile;				 // index of usual visit pattern
  Place * favorite_place[FAVORITE_PLACES];    // list of expected places
  bool on_schedule[FAVORITE_PLACES]; // true iff favorite place is on schedule
  int schedule_updated;			 // date of last schedule update
  bool travel_status;				// true if traveling
  Place ** tmp_favorite_place; // list of favorite places, stored while traveling
  static double age_yearly_mobility_rate[MAX_MOBILITY_AGE + 1];
  static bool is_initialized;

  /**
   * Reads the yearly_mobility_rate_file set in params.def
   * Note: this is not used unless <code>Global::Enable_Mobility</code> is set to <code>true</code>
   */
  void read_init_files();

protected:

  /**
   * Default constructor
   */
  Activities() { }

  /**
   * Place this agent's favorite places into a temporary location
   */
  void store_favorite_places();

  /**
   * Copy the favorite places from the temporary location, then reclaim the allocated memory of the temporary storage
   */
  void restore_favorite_places();
};


#endif // _FRED_ACTIVITIES_H

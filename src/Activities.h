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
  void assign_profile();
  void update(int day);
  void update_infectious_activities(int day);
  void update_susceptible_activities(int day);
  void update_schedule(int day);
  void print_schedule();
  void print();
  Place * get_household() { return favorite_place[HOUSEHOLD_INDEX]; }
  Place * get_neighborhood() { return favorite_place[NEIGHBORHOOD_INDEX]; }
  Place * get_school() { return favorite_place[SCHOOL_INDEX]; }
  Place * get_classroom() { return favorite_place[CLASSROOM_INDEX]; }
  Place * get_workplace() { return favorite_place[WORKPLACE_INDEX]; }
  Place * get_office() { return favorite_place[OFFICE_INDEX]; }
  void assign_school();
  void assign_classroom();
  void assign_workplace();
  void assign_office();
  void update_profile();
  void update_household_mobility();
  void withdraw();
  void addIncidence(int disease, std::vector<int> strains);
  void addPrevalence(int disease, std::vector<int> strains);
  void start_traveling(Person *visited);
  void stop_traveling();
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
  void read_init_files();

protected:
  Activities() { }
  void store_favorite_places();
  void restore_favorite_places();
};


#endif // _FRED_ACTIVITIES_H

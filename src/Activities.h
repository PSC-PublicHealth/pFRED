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

class Activities {
public:
  Activities (Person *person, Place **favorite_place);
  UNIT_TEST_VIRTUAL void assign_profile();
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void update(int day);
  void update_infectious_activities(Date *sim_start_date, int day);
  void update_susceptible_activities(Date *sim_start_date, int day);
  UNIT_TEST_VIRTUAL void update_schedule(Date *sim_start_date, int day);
  UNIT_TEST_VIRTUAL void print_schedule();
  UNIT_TEST_VIRTUAL void print();
  UNIT_TEST_VIRTUAL Place * get_household() { return favorite_place[HOUSEHOLD_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_neighborhood() { return favorite_place[NEIGHBORHOOD_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_school() { return favorite_place[SCHOOL_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_classroom() { return favorite_place[CLASSROOM_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_workplace() { return favorite_place[WORKPLACE_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_office() { return favorite_place[OFFICE_INDEX]; }
  void assign_school();
  void assign_classroom();
  void assign_workplace();
  void assign_office();
  void update_profile();
	
private:
  Person * self;	 // pointer to person using having this activities
  int profile;				 // index of usual visit pattern
  Place * favorite_place[FAVORITE_PLACES];    // list of expected places
  bool on_schedule[FAVORITE_PLACES]; // true iff favorite place is on schedule
  int schedule_updated;			 // date of last schedule update
  
protected:
  Activities() { }
};


#endif // _FRED_ACTIVITIES_H

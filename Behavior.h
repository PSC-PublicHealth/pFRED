/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.h
//

#ifndef _FRED_BEHAVIOR_H
#define _FRED_BEHAVIOR_H

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

class Behavior {
public:
  Behavior (Person *person, Place **favorite_place, int pro);
  UNIT_TEST_VIRTUAL void reset();
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL bool is_on_schedule(int day, int loc, char loctype);
  UNIT_TEST_VIRTUAL void print_schedule();
  UNIT_TEST_VIRTUAL void update_schedule(int day);
  UNIT_TEST_VIRTUAL void get_schedule(int *n, Place **sched);
  UNIT_TEST_VIRTUAL void become_susceptible(int strain);
  UNIT_TEST_VIRTUAL void become_exposed(int strain);
  UNIT_TEST_VIRTUAL void become_infectious(int strain);
  UNIT_TEST_VIRTUAL void become_immune(int strain);
  UNIT_TEST_VIRTUAL void recover(int strain);
  UNIT_TEST_VIRTUAL int compliance_to_vaccination();

  UNIT_TEST_VIRTUAL int get_profile() { return profile; }
  UNIT_TEST_VIRTUAL Place * get_household() { return favorite_place[HOUSEHOLD_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_neighborhood() { return favorite_place[NEIGHBORHOOD_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_school() { return favorite_place[SCHOOL_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_classroom() { return favorite_place[CLASSROOM_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_workplace() { return favorite_place[WORKPLACE_INDEX]; }
  UNIT_TEST_VIRTUAL Place * get_office() { return favorite_place[OFFICE_INDEX]; }
	
private:
  Person * self;	 // pointer to person using having this behavior
  int profile;				 // index of usual visit pattern
  Place * favorite_place[FAVORITE_PLACES];    // list of expected places
  bool on_schedule[FAVORITE_PLACES]; // true iff favorite place is on schedule
  int schedule_updated;			 // date of last schedule update
  
protected:
  Behavior() { }
};


#endif // _FRED_BEHAVIOR_H

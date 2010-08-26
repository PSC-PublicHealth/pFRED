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

class Behavior {
public:
  Behavior (Person *person, Place **favorite_place, int pro);
  UNIT_TEST_VIRTUAL void reset();
	
  UNIT_TEST_VIRTUAL void update(int day);
  UNIT_TEST_VIRTUAL void get_schedule(int *n, Place **sched);
  UNIT_TEST_VIRTUAL int is_on_schedule(int day, int loc);
  UNIT_TEST_VIRTUAL void update_schedule(int day);
  UNIT_TEST_VIRTUAL void print_schedule();
  UNIT_TEST_VIRTUAL void become_susceptible(int strain);
  UNIT_TEST_VIRTUAL void become_infectious(int strain);
  UNIT_TEST_VIRTUAL void become_exposed(int strain);
  UNIT_TEST_VIRTUAL void become_immune(int strain);
  UNIT_TEST_VIRTUAL void recover(int strain);
  UNIT_TEST_VIRTUAL int get_profile() { return profile; }
  UNIT_TEST_VIRTUAL int get_favorite_places() { return favorite_places; }
  UNIT_TEST_VIRTUAL int compliance_to_vaccination();
  UNIT_TEST_VIRTUAL Place * get_household() { return favorite_place[0]; }
  UNIT_TEST_VIRTUAL Place * get_neighborhood() { return favorite_place[1]; }
  UNIT_TEST_VIRTUAL Place * get_school() { return favorite_place[2]; }
  UNIT_TEST_VIRTUAL Place * get_classroom() { return favorite_place[3]; }
  UNIT_TEST_VIRTUAL Place * get_workplace() { return favorite_place[4]; }
  UNIT_TEST_VIRTUAL Place * get_office() { return favorite_place[5]; }
	
private:
  Person * self;	 // pointer to person using having this behavior
  int profile;				 // index of usual visit pattern
  Place ** favorite_place;		      // list of expected places
  int favorite_places;		   // number of places expected to visit
  Place ** schedule;	     // list of place ids actually visited today
  char * on_schedule;	    // 1 = favorite place is on schedule; 0 o.w.
  int scheduled_places;		 // number places actually visited today
  int schedule_updated;			 // date of last schedule update
  
protected:
  Behavior() { }
};


#endif // _FRED_BEHAVIOR_H

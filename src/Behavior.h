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

class Person;
class Place;

class Behavior {
public:
	Behavior (Person *person, Place **favorite_place, int pro);
	virtual void reset();
	
	virtual void update(int day);
	virtual void get_schedule(int *n, Place **sched);
	virtual int is_on_schedule(int day, int loc);
	virtual void update_schedule(int day);
	virtual void print_schedule();
	virtual void become_susceptible(int strain);
	virtual void become_infectious(int strain);
	virtual void become_exposed(int strain);
	virtual void become_immune(int strain);
	virtual void recover(int strain);
	virtual int get_profile() { return profile; }
	virtual int get_favorite_places() { return favorite_places; }
	virtual int compliance_to_vaccination();		// This will currently look up the global vaccine compliance.
  // This is really a place holder for a more sophistocated model.
	virtual Person * get_HoH();
	virtual Place * get_household() { return favorite_place[0]; }
	virtual Place * get_school() { return favorite_place[2]; }
	
private:
	Person * self;				// pointer to person using having this behavior
	int profile;				// index of usual visit pattern
	Place ** favorite_place;	// list of expected places
	int favorite_places;		// number of places expected to visit
	Place ** schedule;			// list of place ids actually visited today
	char * on_schedule;			// 1 = favorite place is on schedule; 0 o.w.
	int scheduled_places;		// number places actually visited today
	int schedule_updated;		// date of last schedule update
};

#endif // _FRED_BEHAVIOR_H
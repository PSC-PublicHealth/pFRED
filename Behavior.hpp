/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.hpp
//

#ifndef _FRED_BEHAVIOR_H
#define _FRED_BEHAVIOR_H

class Behavior {
public:
  Behavior (Person *person, Place *h, Place *n,
	    Place *s, Place *c, Place *w, Place *off, int pro);
  void reset();
  void get_schedule(int *n, int *sched);
  int is_on_schedule(int day, int loc, int sympt);
  void update_schedule(int day, int sympt);
  void print_schedule();
  void become_infectious(int strain, int exposure_date);
  void recover(int strain, int exposure_date);
  int get_favorite_places() { return favorite_places; }

private:
  Person * me;		 // pointer to person using having this behavior
  int id;					// same as person's id
  int profile;				 // index of usual visit pattern
  Place ** favorite_place;		      // list of expected places
  int favorite_places;		   // number of places expected to visit
  Place ** schedule;	     // list of place ids actually visited today
  char * on_schedule;	    // 1 = favorite place is on schedule; 0 o.w.
  int scheduled_places;		 // number places actually visited today
  int schedule_updated;			 // date of last schedule update
};

#endif // _FRED_BEHAVIOR_H


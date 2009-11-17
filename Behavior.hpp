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

#define MAX_PLACES 6

#define DAYS_PER_WEEK 7
#define NO_ROLE 'N'
#define HCW 'H'
#define PATIENT 'P'
#define DOCTOR 'D'

class Place;
class Person;

class Behavior {
public:
  Behavior (Place *h, Place *n, Place *s, Place *c, Place *w, Place *off, int pro);
  void update_schedule(int id, int day, int sympt);
  void get_schedule(int *n, int *sched);
  int is_on_schedule(int id, int day, int loc, int sympt);
  void print_schedule(int id);
  int get_favorite_places() { return favorite_places; }
  void make_susceptible(Person * per, int strain);
  void make_infectious(Person * per, int strain, int exposure_date);
  void make_recovered(Person * per, int strain, int exposure_date);

private:
  int profile;				 // index of usual visit pattern
  Place ** favorite_place;		      // list of expected places
  int favorite_places;		   // number of places expected to visit
  char on_schedule[MAX_PLACES]; // 1 = favorite place is on schedule; 0 o.w.
  Place ** schedule;	     // list of place ids actually visited today
  int scheduled_places;		 // number places actually visited today
  int schedule_updated;			 // date of last schedule update
};

#endif // _FRED_BEHAVIOR_H


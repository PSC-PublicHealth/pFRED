/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: History.h
//

#ifndef _FRED_HISTORY_H
#define _FRED_HISTORY_H

#include <vector>
using namespace std;

// the base-line event storage entity
// (designed for a minimum storage footprint)
struct history_event{
  short int day:14;  // day of simulation when event occurred
  short int type:4;  // enumerated type of event (different enum per context)
  int val1:14;       // small values (like strain)
  int val2:32;       // large values (like person/place ID)
};

// every class having a history must have an enumerated list of event types
enum History_Type_Health {
  HTH_NULL=0,         // every type shall have a NULL, to identify non-init values
  HTH_INFECTION,      // event pertains to infection state
  HTH_INFECTED_PLACE, // where first picked up
  HTH_INFECTED_BY,    // first picked up from whom
  HTH_IMMUNITY,       // event pertains to immunity state
  HTH_ANTIVIRAL,      // event pertains to antiviral state
  HTH_MAX};           // place-holder to identify out-of-range values

// every class having a history must have a dedicated parser
// to interpret the event structure
// (allowing for varying numbers of stored values)
ostream&  history_parser_health(ostream& os, struct history_event &event);

// the History class
// designed to encapsulate all operations on the history_event vector
class History{
public:
  History(void){parser=NULL; this->reset();}
  ~History(void){this->reset();}

  // add an event
  int add(int day, int type, int val1, int val2=0);

  // get the number of events in the history
  inline int get_num_events(void) const {return events.size();}

  // access a single event, by "id" (array index)
  // (returns NULL if none exist)
  inline const struct history_event* get_event_by_id(size_t id) const {
    if (0<=id && id<events.size())
      return (&events[id]);
    return NULL;
  }

  // access a single event, by day number
  // (returns NULL if none exist)
  inline const struct history_event* get_event_by_day(int day) const {
    for (size_t i=0; i<events.size(); i++)
      if (events[i].day == day)
	return (&events[i]);
    return NULL;
  }
  
  // clear out the history list
  // (but do not clear the history parser function)
  inline void reset(void){events.clear();}

  // utility function to dump the entire history contents to STDOUT
  void dump(void);

  // store a parsing function for this History object
  inline void set_parser(ostream& (*parser)(ostream &os, struct history_event &event)) {
    this->parser = parser;
  }

private:
  // the history itslef
  // It is composed of a vector of history_event records.
  vector < struct history_event > events;

  // the history parser function (pointer), stored per-object
  // It returns the ostream object, so that you can inline it.
  // !!! may change format in future revs !!!
  ostream& (*parser)(ostream& os, struct history_event &event);
};

#endif // _FRED_HISTORY_H

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

#define HE_DAY_BITS 14
#define HE_TYPE_BITS 4
#define HE_V2_BITS  14 // put these before V1, to pack out the 1st 32 bits
#define HE_V1_BITS  32 // !!! Note this must remain precisely 32 bits
                       //     or we will be unable to store 'floats' !!!

// the base-line event storage entity
// (designed for a minimum storage footprint)
struct history_event{
  unsigned int day:HE_DAY_BITS;   // day when event occurred
  unsigned int type:HE_TYPE_BITS; // enumerated type of event
                                  // (uses enum History_Type_*)
  unsigned int val2:HE_V2_BITS;   // small values (like strain ID)
  unsigned int val1:HE_V1_BITS;   // normal (larger) values
                                  // (like person/place ID/floats)
};

// every class having a history must have an enumerated list of event types
enum History_Event_Type {
  HE_NULL=0,         // begins with NULL, to identify non-init values

// (Health events)
  HE_INFECTION,      // event pertains to infection state
  HE_INFECTED_PLACE, // where first picked up
  HE_INFECTED_BY,    // first picked up from whom
  HE_IMMUNITY,       // event pertains to immunity state
  HE_ANTIVIRAL,      // event pertains to antiviral state
  HE_MUTATION,       // strain mutated from one to another

// (Behavior events)
  HE_VISIT,          // visited a place
  HE_FAVORITE,       // added a favorite

// (Perception events)
  HE_SUSCEPTIBILITY, // change in perceived susceptibility
  HE_SEVERITY,       // change in perceived severity
  HE_BENEFITS,       // change in perceived benefits
  HE_BARRIERS,       // change in perceived barriers
  HE_EFFICACY,       // change in perceived self-efficacy

// keep a "max" enumerator, to help count these types
  HE_MAX};           // place-holder to identify out-of-range values

// the History class
// designed to encapsulate all operations on the history_event vector
class History{
public:
  // default constructor
  History(void);

  // default destructor
  ~History(void);

  // add an event (at least one integer value)
  int add(unsigned int day, unsigned int type, int val1, int val2=0);

  // add an event (one floating point value)
  int add(unsigned int day, unsigned int type, float val1, int val2=0);
  int add(unsigned int day, unsigned int type, double val1, int val2=0);

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

  // store a parsing function for this History object
  inline void set_parser(ostream& (*parser)(ostream &os, const struct history_event &event)) {
    this->parser = parser;
  }

  // override the default ostream (output) operator
  friend ostream& operator<< (ostream &os, const History &h);

private:
  // the history itslef
  // It is composed of a vector of history_event records.
  vector < struct history_event > events;

  // the history parser function (pointer), stored per-object
  // It returns the ostream object, so that you can inline it.
  // !!! may change format in future revs !!!
  ostream& (*parser)(ostream& os, const struct history_event &event);
};

#endif // _FRED_HISTORY_H

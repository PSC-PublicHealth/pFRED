/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: History.cc
//

#include <iostream>
#include <err.h>
#include <errno.h>

#include "History.h"

// array of descriptive values, corresponding to the above types
// (whatever the user wants to see in a printout)
static const char* history_label[] = {
  "NULL",
  "infection",
  "infected_place",
  "infected_by",
  "immunity",
  "antiviral",
  "mutation",
  "visit",
  "favorite",
  "P_suscept",
  "P_severity",
  "P_benefits",
  "P_barriers",
  "P_efficacy"
};

// default history parser...
ostream&  history_parser_default(ostream& os, const struct history_event &event);

// default constructor
History::History(void){
  // some setup sanity checks:

  cout << "sizeof(struct history_event) = " << sizeof(struct history_event) << endl;

  // 8-byte alignment...
  if (sizeof(struct history_event)%8)
    errx(EINVAL, "struct history_event is not well aligned -- aborting");

  // history_event.val2 storage
  if (32!=HE_V1_BITS)
    errx(EINVAL, "history_event.val2 must have 32 bits, "
	 "to allow for storing float's");

  // make sure the History_Event_Type enum fits in HE_TYPE_BITS
  if (((1ULL<<HE_TYPE_BITS)-1)<HE_MAX)
    errx(EINVAL, "too many health_event types (%d) to fit in %d bits",
	 HE_MAX, HE_TYPE_BITS);

  // enumeration list and its respective static char*
  if (sizeof(history_label)/sizeof(char*) != HE_MAX)
    errx(EINVAL, "History_Event_Type and history_label "
	 "are out of sync");

  // now... on with the "normal" initializations
  parser=history_parser_default; // change it later, if you like
  this->reset();
}

// default destructor
History::~History(void){
  this->reset();
}

// add a single event to the history (integer)
int History::add(unsigned int day, unsigned int type, int val1, int val2){
  struct history_event h;


  // sanity checks (paranoia when using bit-packed values)
  // but don't bail on these, so that we at least get something in the history
  if (((1ULL<<HE_DAY_BITS)-1)<(unsigned long long)day)
    warnx("insufficient storage for day (%d) -- corruption imminent", day);
  if (((1ULL<<HE_V1_BITS)-1)<(unsigned long long)val1)
    warnx("insufficient storage for val1 (%d) -- corruption imminent", val1);
  if (((1ULL<<HE_V2_BITS)-1)<(unsigned long long)val2)
    warnx("insufficient storage for val2 (%d) -- corruption imminent", val2);

  // since parsing is based on type, a messed up type will ruin everything...
  if (HE_MAX<=type){
    warnx("invalid event type (%d)", type);
    return 1;
  }

  // must be okay -- so pack it
  h.day = day;
  h.type = type;
  h.val1 = val1;
  h.val2 = val2;

  //  cout << "packed event: " << h.day << ", " << h.type << ", " << h.val1 << ", " << h.val2 << endl;

  // add it to the vector
  events.push_back(h);

  return 0;
}

// add a single event to the history (floating point)
int History::add(unsigned int day, unsigned int type, float val1, int val2){
  // interpret memory as an int
  int *intp = (int*) (void*) (float*) &val1;

  // and roll on as usual...
  return this->add(day, type, *intp, val2);
}

// ditto... on the double
int History::add(unsigned int day, unsigned int type, double val1, int val2){
  float ff = val1; // drop some resolution, to fit in 32 bit...

  // interpret memory as an int
  int *intp = (int*) (void*) (float*) &ff;

  // and roll on as usual...
  return this->add(day, type, *intp, val2);
}

// write an individual event to the requested ostream
// (no CR/LF)
ostream& history_parser_default(ostream& os, const struct history_event &event){
  if (HE_MAX<=event.type)
    warnx("invalid health event type (%d)", event.type);

  // everybody has a "day"
  os << "day " << event.day << " " << history_label[event.type] << " ";

  // most events are per-strain, except these
  if (!(HE_VISIT==event.type || HE_FAVORITE==event.type))
    os << " strain " << (unsigned int)event.val2 << " ";

  // now, how to interpret the main data
  switch (event.type){
  case HE_INFECTED_PLACE:
  case HE_INFECTED_BY:
  case HE_MUTATION:
  case HE_VISIT:
  case HE_FAVORITE:
    os << event.val1;
    break;
  case HE_INFECTION:
    os << (char)event.val1;
    break;
  case HE_ANTIVIRAL:
    os << "administered by " << event.val1;
    break;
  case HE_IMMUNITY:
    os << (event.val1?"TRUE":"FALSE");
    break;
  case HE_SUSCEPTIBILITY:
  case HE_SEVERITY:
  case HE_BENEFITS:
  case HE_BARRIERS:
  case HE_EFFICACY:
    {
      int ival = event.val1; // get it out of the bit-packed structure
      float* fp = (float*) (void*) (int*) &ival; // interpret memory as float
      os << *fp;
      break;
    }
  default:
    break;
  }

  return os;
}

ostream& operator<< (ostream& os, const History& h){
  // you're DOA if we don't have a history parser
  if (!h.parser){
    warnx("<%s:%d> no parser specified, NOT dumping", __FILE__, __LINE__);
    return os;
  }

  for (size_t i=0; i<h.events.size(); i++){
    (h.parser)(os, h.events[i]) << endl;
  }

  return os;
}


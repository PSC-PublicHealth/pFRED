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
static const char* History_Label_Health[] = {
  "NULL",
  "infection",
  "infected_place",
  "infected_by",
  "immunity",
  "antiviral"};

// add a single event to the history
int History::add(int day, int type, int val1, int val2){
  struct history_event h;

  if (HTH_NULL>type || HTH_MAX<=type){
    warnx("invalid event type (%d)", type);
    return 1;
  }

  h.day = day;
  h.type = type;
  h.val1 = val1;
  h.val2 = val2;

  events.push_back(h);

  return 0;
}

// dump the current history to STDOUT
void History::dump(void){
  if (!parser){
    warnx("<%s:%d> no parser specified, NOT dumping", __FILE__, __LINE__);
    return;
  }
    
  for (size_t i=0; i<events.size(); i++){
    (this->parser)(cout, events[i]) << endl;
  }
}

// write an individual event to the requested ostream
// (no CR/LF)
ostream& history_parser_health(ostream& os, struct history_event &event){
  if (HTH_NULL>event.type || HTH_MAX<=event.type)
    warnx("invalid health event type (%d)", event.type);

  os << "day " << event.day
     << " strain " << event.val1 << " "
     << History_Label_Health[event.type] << " ";
  switch (event.type){
  case HTH_INFECTED_PLACE:
  case HTH_INFECTED_BY:
    os << event.val2 << " ";
    break;
  case HTH_INFECTION:
    os << (char)event.val2 << " ";
    break;
  case HTH_ANTIVIRAL:
    os << "administered by " << event.val2 << " ";
    break;
  case HTH_IMMUNITY:
    os << (event.val2?"TRUE ":"FALSE ");
    break;
  default:
    break;
  }

  return os;
}

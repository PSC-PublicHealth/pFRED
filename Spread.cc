/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Spread.cpp
//

#include "Spread.h"
#include "Strain.h"

#include <stdio.h>
#include <new>
using namespace std;
#include "Random.h"
#include "Params.h"
#include "Person.h"
#include "Global.h"
#include "Infection.h"
#include "Locations.h"
#include "Place.h"

Spread::Spread(Strain *str) {
  char s[80];
  strain = str;
  id = strain->get_id();
  sprintf(s, "index_cases[%d]", id);
  get_param(s, &index_cases);
}

void Spread::reset() {
  exposed.clear();
  infectious.clear();
  attack_rate = 0.0;
  S = E = I = R = 0;
}

void Spread::start_outbreak(Person *pop, int pop_size) {
  // create index cases
  for (int i = 0; i < index_cases; i++) {
    int n = IRAND(0, pop_size-1);
    Infection * infection = new Infection(strain, -1, -1, 'X', 0);
    pop[n].become_exposed(infection);
  }
  E = index_cases;
}

void Spread::update_stats(Person *pop, int pop_size, int day) {
  S = E = I = R = 0;
  for (int p = 0; p < pop_size; p++) {
    char status = pop[p].get_strain_status(id);
    S += (status == 'S');
    E += (status == 'E');
    I += (status == 'I') || (status == 'i');
    R += (status == 'R');
  }
  attack_rate = (100.0*(E+I+R))/pop_size;
}

void Spread::print_stats(int day) {
  int N = S+E+I+R;
  fprintf(Outfp,
	  "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n",
	  day, id, S, E, I, R, N, attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n\n",
	    day, id, S, E, I, R, N, attack_rate);
    fflush(Statusfp);
  }
}

void Spread::insert_into_exposed_list(Person * per) {
  exposed.insert(per);
}

void Spread::insert_into_infectious_list(Person * per) {
  infectious.insert(per);
}

void Spread::remove_from_exposed_list(Person * per) {
  exposed.erase(per);
}

void Spread::remove_from_infectious_list(Person * per) {
  infectious.erase(per);
}

void Spread::update(int day) {
  set <int> places;
  set<Person *>::iterator itr;
  set<int>::iterator it;

  // get list of infectious locations:
  for (itr = infectious.begin(); itr != infectious.end(); itr++) {
    Person * p = *itr;
    if (Verbose > 1) {
      fprintf(Statusfp, "day %d infectious person %d \n", day, p->get_id());
      fflush(Statusfp);
    }
    int n;
    int schedule[100];
    p->update_schedule(day);
    p->get_schedule(&n, schedule);
    for (int j = 0; j < n; j++) {
      int loc = schedule[j];
      Place * place = Loc.get_location(loc);
      if (place->is_open(day) && place->should_be_open(day, id)) {
	places.insert(loc);
      }
    }
  }

  if (Verbose) {
    fprintf(Statusfp, "Number of infectious places = %d\n", (int) places.size());
  }

  // infect visitors to infectious locations:
  for (it = places.begin(); it != places.end(); it++ ) {
    int loc = *it;
    Place * place = Loc.get_location(loc);
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread in location: %d\n", loc); fflush(Statusfp);
    }
    place->spread_infection(day);
  }
}

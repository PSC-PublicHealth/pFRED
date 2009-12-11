/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Place.cc
//

#include "Place.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"
#include "Infection.h"

void Place::setup(int loc_id, char *lab, double lon, double lat, int cont) {
  int strains = Pop.get_strains();
  id = loc_id;
  container = cont;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;

  susceptibles = new (nothrow) vector<Person *> [strains];
  if (susceptibles == NULL) {
    printf("Help! susceptibles allocation failure\n");
    abort();
  }

  infectious = new (nothrow) vector<Person *> [strains];
  if (infectious == NULL) {
    printf("Help! infectious allocation failure\n");
    abort();
  }

  S = new (nothrow) int [strains];
  if (S == NULL) {
    printf("Help! S allocation failure\n");
    abort();
  }

  I = new (nothrow) int [strains];
  if (I == NULL) {
    printf("Help! I allocation failure\n");
    abort();
  }

  Sympt = new (nothrow) int [strains];
  if (Sympt == NULL) {
    printf("Help! Sympt allocation failure\n");
    abort();
  }

  indiv_types = 1;
  reset();
}


void Place::reset() {
  N = 0;
  int strains = Pop.get_strains();
  for (int s = 0; s < strains; s++) {
    susceptibles[s].clear();
    infectious[s].clear();
    Sympt[s] = S[s] = I[s] = 0;
  }
  if (Verbose > 2) {
    printf("reset place: %d\n", id);
    print(0);
    fflush(stdout);
  }
  close_date = INT_MAX;
  open_date = 0;
}

void Place::print(int strain) {
  printf("Place %d label %s type %c ", id, label, type);
  printf("S %d I %d N %d\n", S[strain], I[strain], N);
  fflush(stdout);
}

void Place::add_susceptible(int strain, Person * per) {
  susceptibles[strain].push_back(per);
  S[strain]++;
  if (strain == 0) { N++; }
}


void Place::delete_susceptible(int strain, Person * per) {
  int s = (int) susceptibles[strain].size();
  if (s == 0) {
    printf("Warning! can't delete from empty list! "
	   "Susceptible %i for strain %i place %i\n",
	   per->get_id(), strain, id); 
    return;
  }
  Person * last = susceptibles[strain][s-1];
  
  for (int i = 0; i < s; i++) {
    if (susceptibles[strain][i] == per) {
      susceptibles[strain][i] = last;
      susceptibles[strain].pop_back();
      S[strain]--;
      break;
    }
  }
}

void Place::print_susceptibles(int strain) {
  vector<Person *>::iterator itr;
  for (itr = susceptibles[strain].begin();
       itr != susceptibles[strain].end(); itr++) {
    printf(" %d", (*itr)->get_id());
  }
  printf("\n");
}


void Place::add_infectious(int strain, Person * per) {
  infectious[strain].push_back(per);
  I[strain]++;
  if (per->get_strain_status(strain) == 'I') {
    Sympt[strain]++;
  }
}

void Place::delete_infectious(int strain, Person * per) {
  int s = (int) infectious[strain].size();
  Person * last = infectious[strain][s-1];
  for (int i = 0; i < s; i++) {
    if (infectious[strain][i] == per) {
      infectious[strain][i] = last;
      infectious[strain].pop_back();
      I[strain]--;
      if (per->get_strain_status(strain)=='I') {
	Sympt[strain]--;
      }
      break;
    }
  }
}

void Place::print_infectious(int strain) {
  vector<Person *>::iterator itr;
  for (itr = infectious[strain].begin();
       itr != infectious[strain].end(); itr++) {
    printf(" %d", (*itr)->get_id());
  }
  printf("\n");
}

void Place::spread_infection(int day) {
  vector<Person *>::iterator itr;
  int strains = Pop.get_strains();
  for (int s = 0; s < strains; s++) {
    Strain * strain = Pop.get_strain(s);
    if (Verbose > 1) { print(s); }
    if (N < 2) return;
    if (S[s] == 0) continue;

    // expected number of susceptible contacts for each infectious person
    double contacts = get_contacts_per_day(s) * ((double) S[s]) / ((double) (N-1));
    if (Verbose > 1) {
      printf("expected suscept contacts = %f\n", contacts);
      fflush(stdout);
    }
    
    // expected upper bound on number of contacts resulting in infection (per infective)
    contacts *= strain->get_transmissibility();
    if (Verbose > 1) {
      printf("beta = %f\n", strain->get_transmissibility());
      printf("effective contacts = %f\n", contacts);
      fflush(stdout);
    }

    for (itr = infectious[s].begin(); itr != infectious[s].end(); itr++) {
      Person * infector = *itr;			// infectious indiv

      if (Verbose > 1) { printf("is infector %d here?\n", infector->get_id()); }

      // skip if this infector did not visit today
      if (!infector->is_on_schedule(day, id)) continue;

      if (Verbose > 1) { printf("infected = %d\n", infector->get_id()); }

      // reduce number of infective contact events by my infectivity
      double my_contacts = contacts * infector->get_infectivity(s);
      if (Verbose > 1) {
	printf("infectivity = %f\n", infector->get_infectivity(s));
	printf("my effective contacts = %f\n", my_contacts);
	fflush(stdout);
      }

      // randomly round off the expected value of the contact counts
      int contact_count = (int) my_contacts;
      double r = RANDOM();
      if (r < my_contacts - contact_count) contact_count++;
      if (Verbose > 1) {
	printf("my contact_count = %d  r = %f\n", contact_count, r);
	fflush(stdout);
      }

      // get susceptible target for each contact resulting in infection
      for (int c = 0; c < contact_count; c++) {
	double r = RANDOM();
	int pos = (int) (r*S[s]);
	Person * infectee = susceptibles[s][pos];
	if (Verbose > 1) {
	  printf("my possible victim = %d  r = %f  pos = %d  S[d] = %d\n",
		 infectee->get_id(), r, pos, S[s]);
	}
	if (strain->attempt_infection(infector, infectee, this, day)) {
	  if (Verbose > 1) {
	    printf("infection from %d to %d\n",
		   infector->get_id(),infectee->get_id());
	  }
	} else {
	  if (Verbose > 1) { 
	    printf("no infection\n");
	  }
	}
      } // end contact loop
    } // end infectious list loop
  }  // end strain loop
}

int Place::is_open(int day) {
  if (container > 0) {
    return Loc.get_open_status(container, day);
  }
  else {
    return (day < close_date || open_date <= day);
  }
}


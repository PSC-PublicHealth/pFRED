/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Place.cpp
//

#include "Place.hpp"
#include "Global.hpp"
#include "Params.hpp"
#include "Random.hpp"
#include "Population.hpp"
#include "Strain.hpp"

void Place::setup(int loc, char *lab, double lon, double lat, int cont) {

  int strains = Strain::get_strains();
  id = loc;
  container = cont;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;

  susceptibles = new (nothrow) vector<int> [strains];
  if (susceptibles == NULL) {
    printf("Help! susceptibles allocation failure\n");
    abort();
  }

  infectious = new (nothrow) vector<int> [strains];
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
  int strains = Strain::get_strains();
  for (int d = 0; d < strains; d++) {
    susceptibles[d].clear();
    infectious[d].clear();
    Sympt[d] = S[d] = I[d] = 0;
  }
  if (Verbose > 2) {
    printf("reset place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}

void Place::print(int strain) {
  printf("Place %d label %s type %c ", id, label, type);
  printf("S %d I %d N %d\n", S[strain], I[strain], N);
  fflush(stdout);
}

void Place::add_susceptible(int strain, int per) {
  susceptibles[strain].push_back(per);
  S[strain]++;
  if (strain == 0) { N++; }
}


void Place::delete_susceptible(int strain, int per) {
  int s = (int) susceptibles[strain].size();
  int last = susceptibles[strain][s-1];
  for (int i = 0; i < s; i++) {
    if (susceptibles[strain][i] == per) {
      susceptibles[strain][i] = last;
      susceptibles[strain].pop_back();
    }
  }
  S[strain]--;
}

void Place::print_susceptibles(int strain) {
  vector<int>::iterator itr;
  for (itr = susceptibles[strain].begin();
       itr != susceptibles[strain].end(); itr++) {
    printf(" %d", *itr);
  }
  printf("\n");
}


void Place::add_infectious(int strain, int per) {
  infectious[strain].push_back(per);
  I[strain]++;
  if (Pop.get_strain_status(per, strain) == 'I') {
    Sympt[strain]++;
  }
}

void Place::delete_infectious(int strain, int per) {
  int s = (int) infectious[strain].size();
  int last = infectious[strain][s-1];
  for (int i = 0; i < s; i++) {
    if (infectious[strain][i] == per) {
      infectious[strain][i] = last;
      infectious[strain].pop_back();
    }
  }
  I[strain]--;
  if (Pop.get_strain_status(per, strain)=='I') {
    Sympt[strain]--;
  }
}

void Place::print_infectious(int strain) {
  vector<int>::iterator itr;
  for (itr = infectious[strain].begin();
       itr != infectious[strain].end(); itr++) {
    printf(" %d", *itr);
  }
  printf("\n");
}

void Place::spread_infection(int day) {
  vector<int>::iterator itr;

  int strains = Strain::get_strains();
  for (int d = 0; d < strains; d++) {
    if (Verbose > 1) { print(d); }
    if (N < 2) return;
    if (S[d] == 0) continue;

    // expected number of susceptible contacts for each infectious person
    double contacts = get_contacts_per_day(d) * ((double) S[d]) / ((double) (N-1));
    if (Verbose > 1) {
      printf("expected suscept contacts = %f\n", contacts);
      fflush(stdout);
    }
    
    // expected u.b. on number of contacts resulting in infection (per infective)
    contacts *= Strain::get_beta(d);
    if (Verbose > 1) {
      printf("beta = %f\n", Strain::get_beta(d));
      printf("effective contacts = %f\n", contacts);
      fflush(stdout);
    }

    for (itr = infectious[d].begin(); itr != infectious[d].end(); itr++) {
      int i = *itr;				// infectious indiv

      if (Verbose > 1) { printf("infectious %d here?\n", i); }

      // skip if this infected did not visit today
      if (!Pop.is_place_on_schedule_for_person(i, day, id)) continue;

      if (Verbose > 1) { printf("infected = %d\n", i); }

      // reduce number of infective contact events by my infectivity
      double my_contacts = contacts * Pop.get_infectivity(i,d);
      if (Verbose > 1) {
	printf("infectivity = %f\n", Pop.get_infectivity(i,d));
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
	// int pos = IRAND(0, S[d]-1);
	int pos = (int) (r*S[d]);
	int s = susceptibles[d][pos];
	if (Verbose > 1) { printf("my possible victim = %d  r = %f  pos = %d  S[d] = %d\n",s, r, pos, S[d]); }

	// is the victim here today, and still susceptible?
	if (Pop.is_place_on_schedule_for_person(s,day,id) && Pop.get_strain_status(s,d) == 'S') {

	  // compute transmission prob for this type of individuals
	  double transmission_prob = get_transmission_prob(d,i,s);

	  // get the victim's susceptibility
	  double susceptibility = Pop.get_susceptibility(s,d);

	  double r = RANDOM();
	  if (r < transmission_prob*susceptibility) {
	    if (Verbose > 1) { printf("infection from %d to %d  r = %f\n",i,s,r); }
	    Pop.make_exposed(s, d, i, id, type, day);
	    Pop.add_infectee(i,d);
	  }
	  else {
	    if (Verbose > 1) { printf("no infection\n"); }
	  }
	}
	else {
	  if (Verbose > 1) {
	    printf("victim not here today, or not still susceptible\n");
	    printf ("%s here", Pop.is_place_on_schedule_for_person(s,day,id)? "is": "is not");
	    printf("strain status = %c\n", Pop.get_strain_status(s,d));
	    fflush(stdout);
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


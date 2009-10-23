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
extern int Verbose;


void Place::setup(int loc, char *lab, double lon, double lat, int cont) {

  int diseases = get_diseases();
  id = loc;
  container = cont;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;

  susceptibles = new (nothrow) vector<int> [diseases];
  if (susceptibles == NULL) {
    printf("Help! susceptibles allocation failure\n");
    abort();
  }

  infectious = new (nothrow) vector<int> [diseases];
  if (infectious == NULL) {
    printf("Help! infectious allocation failure\n");
    abort();
  }

  S = new (nothrow) int [diseases];
  if (S == NULL) {
    printf("Help! S allocation failure\n");
    abort();
  }

  I = new (nothrow) int [diseases];
  if (I == NULL) {
    printf("Help! I allocation failure\n");
    abort();
  }

  Sympt = new (nothrow) int [diseases];
  if (Sympt == NULL) {
    printf("Help! Sympt allocation failure\n");
    abort();
  }

  indiv_types = 1;
  reset();
}


void Place::reset() {
  N = 0;
  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {
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

void Place::print(int dis) {
  printf("Place %d label %s type %c\n", id, label, type);
  //   printf("contacts %d contact_p %f S %d I %d N %d\n",
  // contacts_per_day[dis], contact_prob[dis], S[dis], I[dis], N);
  fflush(stdout);
}

void Place::add_susceptible(int dis, int per) {
  susceptibles[dis].push_back(per);
  S[dis]++;
  if (dis == 0) { N++; }
}


void Place::delete_susceptible(int dis, int per) {
  int s = (int) susceptibles[dis].size();
  int last = susceptibles[dis][s-1];
  for (int i = 0; i < s; i++) {
    if (susceptibles[dis][i] == per) {
      susceptibles[dis][i] = last;
      susceptibles[dis].pop_back();
    }
  }
  S[dis]--;
}

void Place::print_susceptibles(int dis) {
  vector<int>::iterator itr;
  for (itr = susceptibles[dis].begin();
       itr != susceptibles[dis].end(); itr++) {
    printf(" %d", *itr);
  }
  printf("\n");
}


void Place::add_infectious(int dis, int per) {
  infectious[dis].push_back(per);
  I[dis]++;
  if (get_disease_status(per, dis) == 'I') {
    Sympt[dis]++;
  }
}

void Place::delete_infectious(int dis, int per) {
  int s = (int) infectious[dis].size();
  int last = infectious[dis][s-1];
  for (int i = 0; i < s; i++) {
    if (infectious[dis][i] == per) {
      infectious[dis][i] = last;
      infectious[dis].pop_back();
    }
  }
  I[dis]--;
  if (get_disease_status(per, dis)=='I') {
    Sympt[dis]--;
  }
}

void Place::print_infectious(int dis) {
  vector<int>::iterator itr;
  for (itr = infectious[dis].begin();
       itr != infectious[dis].end(); itr++) {
    printf(" %d", *itr);
  }
  printf("\n");
}

void Place::spread_infection(int day) {
  vector<int>::iterator itr;

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {
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
    contacts *= get_beta(d);
    if (Verbose > 1) {
      printf("beta = %f\n", get_beta(d));
      printf("effective contacts = %f\n", contacts);
      fflush(stdout);
    }

    for (itr = infectious[d].begin(); itr != infectious[d].end(); itr++) {
      int i = *itr;				// infectious indiv

      // skip if this infected did not visit today
      if (!is_place_on_schedule_for_person(i, day, id)) continue;

      if (Verbose > 1) { printf("infected = %d\n", i); }

      // reduce number of infective contact events by my infectivity
      double my_contacts = contacts * get_infectivity(i,d);
      if (Verbose > 1) {
	printf("my effective contacts = %f\n", my_contacts);
	fflush(stdout);
      }

      // randomly round off the expected value of the contact counts
      int contact_count = (int) my_contacts;
      if (RANDOM() < my_contacts - contact_count) contact_count++;
      if (Verbose > 1) {
	printf("my contact_count = %d\n", contact_count);
	fflush(stdout);
      }

      // get susceptible target for each contact resulting in infection
      for (int c = 0; c < contact_count; c++) {
	int pos = IRAND(0, S[d]-1);
	int s = susceptibles[d][pos];
	if (Verbose > 1) { printf("my possible victim = %d\n",s); }

	// is the victim here today, and still susceptible?
	if (is_place_on_schedule_for_person(s,day,id) && get_disease_status(s,d) == 'S') {

	  // compute transmission prob for this type of individuals
	  double transmission_prob = get_transmission_prob(d,i,s);

	  // get the victim's susceptibility
	  double susceptibility = get_susceptibility(s,d);

	  if (RANDOM() < transmission_prob*susceptibility) {
	    if (Verbose > 1) { printf("infection from %d to %d\n",i,s); }
	    make_exposed(s, d, i, id, type, day);
	    add_infectee(i,d);
	  }
	  else {
	    if (Verbose > 1) { printf("no infection\n"); }
	  }
	}
	else {
	  if (Verbose > 1) {
	    printf("victim not here today, or not still susceptible\n");
	    printf ("%s here", is_place_on_schedule_for_person(s,day,id)? "is": "is not");
	    printf("disease status = %c\n", get_disease_status(s,d));
	    fflush(stdout);
	  }
	}
      } // end contact loop
    } // end infectious list loop
  }  // end disease loop
}

int Place::is_open(int day) {
  int get_open_status(int,int);
  if (container > 0) {
    return get_open_status(container, day);
  }
  else {
    return (day < close_date || open_date <= day);
  }
}


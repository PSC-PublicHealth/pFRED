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
#include "Disease.h"
#include "Infection.h"

void Place::setup(int loc_id, const char *lab, double lon, double lat, Place* cont, Population *pop) {
  population = pop;
  diseases = population->get_diseases();
  id = loc_id;
  container = cont;
  strcpy(label, lab);
  longitude = lon;
  latitude = lat;
  N = 0;
  
  // allocate disease-related memory
  susceptibles = new (nothrow) vector<Person *> [diseases];
  assert (susceptibles != NULL);
  infectious = new (nothrow) vector<Person *> [diseases];
  assert (infectious != NULL);
  S = new (nothrow) int [diseases];
  assert (S != NULL);
  I = new (nothrow) int [diseases];
  assert (I != NULL);
  Sympt = new (nothrow) int [diseases];
  assert (Sympt != NULL);
  cases = new (nothrow) int [diseases];
  assert (cases != NULL);
  total_cases = new (nothrow) int [diseases];
  assert (total_cases != NULL);
  deaths = new (nothrow) int [diseases];
  assert (deaths != NULL);
  total_deaths = new (nothrow) int [diseases];
  assert (total_deaths != NULL);
  reset();
}


void Place::reset() {
  for (int s = 0; s < diseases; s++) {
    // printf("PLACE %d  %s  reserving %d\n",id,label,N);
    susceptibles[s].reserve(N);
    infectious[s].reserve(N);
    Sympt[s] = S[s] = I[s] = 0;
    total_cases[s] = total_deaths[s] = 0;
  }
  close_date = INT_MAX;
  open_date = 0;
  if (Verbose > 2) {
    printf("reset place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}

void Place::update(int day) {
  for (int s = 0; s < diseases; s++) {
    cases[s] = deaths[s] = 0;
    susceptibles[s].clear();
    infectious[s].clear();
    Sympt[s] = S[s] = I[s] = 0;
  }
}

void Place::print(int disease) {
  printf("Place %d label %s type %c ", id, label, type);
  printf("S %d I %d N %d\n", S[disease], I[disease], N);
  fflush(stdout);
}

void Place::add_person(Person * per) {
  N++;
}


void Place::add_susceptible(int disease, Person * per) {
  susceptibles[disease].push_back(per);
  S[disease]++;
  assert (S[disease] == static_cast <int> (susceptibles[disease].size()));
}


void Place::add_infectious(int disease, Person * per, char status) {
  infectious[disease].push_back(per);
  I[disease]++;
  assert(I[disease] == static_cast <int> (infectious[disease].size()));
  if (status == 'I') {
    Sympt[disease]++;
    cases[disease]++;
    total_cases[disease]++;
  }
  if (I[disease]==1) {
    Disease * dis = population->get_disease(disease);
    dis->add_infectious_place(this, type);
  }
}

void Place::print_susceptibles(int disease) {
  vector<Person *>::iterator itr;
  for (itr = susceptibles[disease].begin();
       itr != susceptibles[disease].end(); itr++) {
    printf(" %d", (*itr)->get_id());
  }
  printf("\n");
}


void Place::print_infectious(int disease) {
  vector<Person *>::iterator itr;
  for (itr = infectious[disease].begin();
       itr != infectious[disease].end(); itr++) {
    printf(" %d", (*itr)->get_id());
  }
  printf("\n");
}

int Place::is_open(int day) {
  if (container) {
    return container->is_open(day);
  } else {
    return (day < close_date || open_date <= day);
  }
}

void Place::spread_infection(int day, int s) {
  if (Verbose > 1) { print(s); }
  if (N < 2) return;
  if (S[s] == 0) return;
  if (is_open(day) == false) return;
  if (should_be_open(day, s) == false) return;
	
  vector<Person *>::iterator itr;
  Disease * disease = population->get_disease(s);
  double beta = disease->get_transmissibility();

  // expected number of susceptible contacts for each infectious person
  double contacts = get_contacts_per_day(s) * ((double) S[s]) / ((double) (N-1));
  if (Verbose > 1) {
    printf("Disease %i, expected suscept contacts = %.3f * %i / %i = %f\n",
           s, get_contacts_per_day(s), S[s], (N-1), contacts);
    fflush(stdout);
  }
  
  // expected upper bound on number of contacts resulting in infection
  // (per infective)
  contacts *= beta;
  if (Verbose > 1) {
    printf("beta = %.10f\n", beta);
    printf("effective contacts = %f\n", contacts);
    fflush(stdout);
  }
	
  for (itr = infectious[s].begin(); itr != infectious[s].end(); itr++) {
    int number_susceptibles = S[s];
    if (number_susceptibles == 0) break;
    Person * infector = *itr;			// infectious indiv
    assert(infector->get_disease_status(s) == 'I' ||
	   infector->get_disease_status(s) == 'i');
		
    // reduce number of infective contact events by my infectivity
    double my_contacts = contacts * infector->get_infectivity(s);
    if (Verbose > 1) {
      printf("infectivity = %f so ", infector->get_infectivity(s));
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
		
    // get a susceptible target for each contact resulting in infection
    for (int c = 0; c < contact_count; c++) {
      // This check for S[s] == 0 looks redundant, because it's also done
      // in the outer loop - but this inner loop can itself change S[s]
      // so we need to check within the loop too.
      if (S[s] == 0) { break; }

      double r = RANDOM();
      int pos = (int) (r*S[s]);
      Person * infectee = susceptibles[s][pos];
      if (Verbose > 1) {
	printf("my possible victim = %d  r = %f  pos = %d  S[%d] = %d\n",
               infectee->get_id(), r, pos, s, S[s]);
	fflush(stdout);
      }

      // is the victim still susceptible?
      if (infectee->is_susceptible(s)) {
	if (Verbose > 1) { printf("Victim is susceptible\n"); }
    
	// get the victim's susceptibility
	double transmission_prob = get_transmission_prob(s, infector, infectee);
	double susceptibility = infectee->get_susceptibility(s);
	if (Verbose > 1) {
	  printf("trans_prob = %f  susceptibility = %f\n",
		 transmission_prob, susceptibility);
	}
    
	// attempt transmission
	double r = RANDOM();
	if (r < transmission_prob*susceptibility) {
	  if (Verbose > 1) { printf("transmission succeeded: r = %f\n", r); }
	  Infection * infection = new Infection(disease, infector, infectee, this, day);
	  infectee->become_exposed(infection);
	  infector->add_infectee(s);
	  if (Verbose > 1) {
	    if (infector->get_exposure_date(s) == 0) {
	      printf("SEED infection day %i from %d to %d\n",
		     day, infector->get_id(),infectee->get_id());
	    } else {
	      printf("infection day %i of disease %i from %d to %d\n",
		     day, s, infector->get_id(),infectee->get_id());
	    }
	  }
	}
	else {
	  if (Verbose > 1) { printf("transmission failed: r = %f\n", r); }
	}
      }
    } // end contact loop
  } // end infectious list loop

  if (Verbose > 1) { fflush(stdout); }
}


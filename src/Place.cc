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
#include "Transmission.h"

void Place::setup(int loc_id, const char *lab, double lon, double lat, Place* cont, Population *pop) {
  population = pop;
  diseases = population->get_diseases();
  id = loc_id;
  container = cont;
  strcpy(label, lab);
  longitude = lon;
  latitude = lat;
  
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
  incidence.resize(diseases);
  prevalence.resize(diseases);
  N = 0;
}

void Place::print_stats(int day, int disease) {
  /*
   * The output format for this needs to be changed and 
   * it produces an insane amount of output in the current format
   *

   // print out incidence and prevalence values
   map<int, int> :: iterator it;
   fprintf(Incfp, "%d %s ", day, label);
   int tot = 0;
   for(it = incidence[disease].begin(); it != incidence[disease].end(); it++){
   tot += it->second;
   }
   fprintf(Incfp, "%d ", tot);

   for(it = incidence[disease].begin(); it != incidence[disease].end(); it++){
   fprintf(Incfp, " %d %d ", it->first, it->second);
   }
   fprintf(Incfp, "\n");

   fprintf(Prevfp, "%d %s ", day, label);
   for(it = incidence[disease].begin(); it != incidence[disease].end(); it++){
   tot += it->second;
   }
   fprintf(Prevfp, "%d ", tot);

   for(it = prevalence[disease].begin(); it != prevalence[disease].end(); it++){
   fprintf(Prevfp, "%d %d ", it->first, it->second);
   }
   fprintf(Prevfp, "\n");
  */
}


void Place::prepare() {
  for (int s = 0; s < diseases; s++) {
    susceptibles[s].reserve(N);
    infectious[s].reserve(N);
    total_cases[s] = total_deaths[s] = 0;
  }
  update(0);
  open_date = 0;
  close_date = INT_MAX;
  if (Global::Verbose > 2) {
    printf("prepare place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}

void Place::update(int day) {
  for (int s = 0; s < diseases; s++) {
    // if(day > 0) print_stats(day, s);
    cases[s] = deaths[s] = 0;
    susceptibles[s].clear();
    infectious[s].clear();
    Sympt[s] = S[s] = I[s] = 0;
    incidence[s].clear();
  }
}

void Place::print(int disease) {
  printf("Place %d label %s type %c ", id, label, type);
  printf("S %d I %d N %d\n", S[disease], I[disease], N);
  fflush(stdout);
}

void Place::enroll(Person * per) {
  N++;
}

void Place::unenroll(Person * per) {
  N--;
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

bool Place::is_open(int day) {
  if (container) {
    return container->is_open(day);
  } else {
    return (day < close_date || open_date <= day);
  }
}

void Place::spread_infection(int day, int s) {

  // the number of possible infectees per infector is max of (N-1) and S[s]
  // where N is the capacity of this place and S[s] is the number of current susceptibles
  // visiting this place.  S[s] might exceed N if we have some ad hoc visitors,
  // since N is estimated only at startup.
  int number_targets = (N-1 > S[s]? N-1 : S[s]);

  if (Global::Verbose > 1) { print(s); }
  if (number_targets == 0) return;
  if (is_open(day) == false) return;
  if (should_be_open(day, s) == false) return;
	
  vector<Person *>::iterator itr;
  Disease * disease = population->get_disease(s);

  // expected number of susceptible contacts for each infectious person
  // OLD: double contacts = get_contacts_per_day(s) * ((double) S[s]) / ((double) (N-1));
  double contacts = get_contacts_per_day(s) * disease->get_transmissibility();
  if (Global::Verbose > 1) {
    printf("Disease %d, expected contacts = %f\n", s, contacts);
    fflush(stdout);
  }

  for (itr = infectious[s].begin(); itr != infectious[s].end(); itr++) {
    Person * infector = *itr;			// infectious indiv
    assert(infector->get_disease_status(s)=='I'||infector->get_disease_status(s)=='i');
		
    // reduce number of infective contacts by infector's infectivity
    double infector_contacts = contacts * infector->get_infectivity(s, day);
    if (Global::Verbose > 1) {
      printf("infectivity = %f so ", infector->get_infectivity(s, day));
      printf("infector's effective contacts = %f\n", infector_contacts);
      fflush(stdout);
    }
		
    // randomly round off the expected value of the contact counts
    int contact_count = (int) infector_contacts;
    double r = RANDOM();
    if (r < infector_contacts - contact_count) contact_count++;
    if (Global::Verbose > 1) {
      printf("infector contact_count = %d  r = %f\n", contact_count, r);
      fflush(stdout);
    }
		
    // get a susceptible target for each contact resulting in infection
    for (int c = 0; c < contact_count; c++) {

      // select a target infectee with replacement, including all possible visitors
      int pos = IRAND(0,number_targets-1);
      if (pos > S[s]-1) continue; // target is not one of the susceptibles present

      // at this point we have a susceptible target:
      Person * infectee = susceptibles[s][pos];
      if (Global::Verbose > 1) {
	printf("possible infectee = %d  pos = %d  S[%d] = %d\n",
               infectee->get_id(), pos, s, S[s]);
	fflush(stdout);
      }

      // is the target still susceptible?
      if (infectee->is_susceptible(s)) {
	if (Global::Verbose > 1) { printf("Victim is susceptible\n"); }
    
	// get the transmission probs for this infector/infectee pair
	double transmission_prob = get_transmission_prob(s, infector, infectee);
	double susceptibility = infectee->get_susceptibility(s);
	if (Global::Verbose > 1) {
	  printf("trans_prob = %f  susceptibility = %f\n",
		 transmission_prob, susceptibility);
	  fflush(stdout);
	}
    
	// attempt transmission
	double r = RANDOM();
	if (r < transmission_prob*susceptibility) {
	  if (Global::Verbose > 1) {
	    printf("transmission succeeded: r = %f  prob = %f\n",
		   r, transmission_prob*susceptibility);
	    fflush(stdout);
	  }

	  // successful transmission; create a new infection in infectee
	  // Infection * infection = new Infection(disease, infector, infectee, this, day);
	  // infectee->become_exposed(infection);
          // infector->add_infectee(s);
          Transmission *transmission = new Transmission(infector, this, day);
          infector->infect(infectee, s, transmission);

	  if (Global::Verbose > 1) {
	    if (infector->get_exposure_date(s) == 0) {
	      printf("SEED infection day %i from %d to %d\n",
		     day, infector->get_id(),infectee->get_id());
	    } else {
	      printf("infection day %i of disease %i from %d to %d\n",
		     day, s, infector->get_id(),infectee->get_id());
	    }
	    fflush(stdout);
	  }
	}	else {
	  if (Global::Verbose > 1) {
	    printf("transmission failed: r = %f  prob = %f\n",
		   r, transmission_prob*susceptibility);
	    fflush(stdout);
	  }
        }
      } // end of susceptible infectee
    } // end contact loop
  } // end infectious list loop
}

void Place::modifyIncidenceCount(int disease, vector<int> strains, int incr) {
  map<int, int>& inc = incidence[disease];
  for(int s=0; s < (int) strains.size(); s++) {
    if(inc.find(strains[s]) == inc.end()) inc[strains[s]] = 0;
    inc[strains[s]] = inc[strains[s]] + 1;
  }
}

void Place::modifyPrevalenceCount(int disease, vector<int> strains, int incr) {
  map<int, int>& prev = prevalence[disease];
  for(int s=0; s < (int) strains.size(); s++) {
    prev[strains[s]] += 1;
  }
}


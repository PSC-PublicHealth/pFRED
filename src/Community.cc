/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Community.cc
//

#include "Community.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Strain.h"

double * Community_contacts_per_day;
double *** Community_contact_prob;
int Community_parameters_set = 0;

Community::Community(int loc, const char *lab, double lon, double lat, Place *container, Population * pop) {
  type = COMMUNITY;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_strains());
}

void Community::get_parameters(int strains) {
  char param_str[80];
  
  if (Community_parameters_set) return;
  
  Community_contacts_per_day = new double [ strains ];
  Community_contact_prob = new double** [ strains ];
  
  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "community_contacts[%d]", s);
    get_param((char *) param_str, &Community_contacts_per_day[s]);
    // printf("Community_contacts_per_day[%d]= %f\n",d,Community_contacts_per_day[s]);
    sprintf(param_str, "community_prob[%d]", s);
    n = get_param_matrix(param_str, &Community_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nCommunity_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Community_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  Community_parameters_set = 1;
}

int Community::get_group_type(int strain, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Community::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group_type(strain, i);
  int col = get_group_type(strain, s);
  double tr_pr = Community_contact_prob[strain][row][col];
  return tr_pr;
}

double Community::get_contacts_per_day(int strain) {
  return Community_contacts_per_day[strain];
}

void Community::spread_infection_in_community(int day, int s) {
  return;
  vector<Person *>::iterator itr;
  Strain * strain = population->get_strain(s);
  if (Verbose > 1) { print(s); }
  if (N < 2) return;
  if (S[s] == 0) return;
	
  // expected number of susceptible contacts for each infectious person
  double contacts = get_contacts_per_day(s) * ((double) S[s]) / ((double) (N-1));
  if (Verbose > 1) {
    printf("Strain %i, expected suscept contacts = %.3f * %i / %i = %f\n",
           s, get_contacts_per_day(s), S[s], (N-1), contacts);
    fflush(stdout);
  }
  
  // expected upper bound on number of contacts resulting in infection
  // (per infective)
  contacts *= strain->get_transmissibility();
  if (Verbose > 1) {
    printf("beta = %.10f\n", strain->get_transmissibility());
    printf("effective contacts = %f\n", contacts);
    fflush(stdout);
  }
	
  if (contacts == 0.0) { return; }

  for (itr = infectious[s].begin(); itr != infectious[s].end(); itr++) {
    if (S[s] == 0) break;
    Person * infector = *itr;			// infectious indiv
    if (Debug && infector->get_strain_status(s) != 'I' &&
        infector->get_strain_status(s) != 'i') {
      printf("Non-infectious person on infectious list: person %i day %i\n",
             infector->get_id(), day);
      abort();
    }
		
    // skip if this infector did not visit today
    if (Verbose > 1) { printf("Is infector %d here?  ", infector->get_id()); }
    if (!infector->is_on_schedule(day, id)) {  
      if (Verbose > 1) { printf("No\n"); }
      continue;
    }
    if (Verbose > 1) { printf("Yes\n"); }
		
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
		
    // get susceptible target for each contact resulting in infection
    for (int c = 0; c < contact_count; c++) {
      // This check for S[s] == 0 looks redundant, because it's also done
      // in the outer loop - but this inner loop can itself change S[s]
      // so we need to check within the loop too.
      if (S[s] == 0)
	break;
      double r = RANDOM();
      int pos = (int) (r*S[s]);
      Person * infectee = susceptibles[s][pos];
      if (Debug && infectee->get_strain_status(s) != 'S') {
	printf("Non-susceptible person on susceptible list for "
               "strain %i: person %i day %i\n",
               s, infectee->get_id(), day);
	abort();
      }
      if (Verbose > 1) {
	printf("my possible victim = %d  prof = %d r = %f  pos = %d  S[%d] = %d\n",
               infectee->get_id(), infectee->get_behavior()->get_profile(), r, pos, s, S[s]);
      }

      int strain_id = strain->get_id();

      // is the victim here today, and still susceptible?
      if (infectee->is_on_schedule(day, id) &&
	  infectee->get_strain_status(strain_id) == 'S') {
	if (Verbose > 1) { printf("Victim is here and is susceptible\n"); }
    
	// get the victim's susceptibility
	double susceptibility;
	double transmission_prob;
	transmission_prob = get_transmission_prob(strain_id, infector, infectee);
	susceptibility = infectee->get_susceptibility(strain_id);
	if (Verbose > 1) {
	  printf("trans_prob = %f  susceptibility = %f\n",
		 transmission_prob, susceptibility);
	}
    
	double r = RANDOM();
	if (r < transmission_prob*susceptibility) {
	  if (Verbose > 1) { printf("transmission succeeded: r = %f\n", r); }
	  Infection* i = new Infection(strain, infector, infectee, this, day);
	  infectee->become_exposed(i);
	  infector->add_infectee(strain_id);
	  if (Verbose > 1) {
	    if (infector->get_exposure_date(s) == 0) {
	      printf("SEED infection day %i from %d to %d\n",
		     day, infector->get_id(),infectee->get_id());
	    } else {
	      printf("infection day %i of strain %i from %d to %d\n",
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

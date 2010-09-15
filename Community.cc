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

Community * community;
double * Community_contacts_per_day;
double * Community_limit;
int Community_limit_trials;
double *** Community_contact_prob;
int Community_parameters_set = 0;

Community::Community(int loc, const char *lab, double lon, double lat, Place *container, Population * pop) {
  int strains = pop->get_strains();
  type = COMMUNITY;
  groups = 2;
  get_parameters(strains);

  // allocate strain-related memory
  infectious_list = new (nothrow) set<Person *> * [strains];
  assert (infectious_list != NULL);

  for (int s = 0; s < strains; s++) {
    infectious_list[s] = new (nothrow) set<Person *> [groups];
    assert (infectious_list[s] != NULL);
  } 

  setup(loc, lab, lon, lat, container, pop);
}

void Community::get_parameters(int strains) {
  char param_str[80];
  if (Community_parameters_set) return;
  Community_contacts_per_day = new double [ strains ];
  Community_contact_prob = new double** [ strains ];
  Community_limit = new double [ strains ];
  for (int s = 0; s < strains; s++) {
    int n;
    sprintf(param_str, "community_limit_trials");
    get_param((char *) param_str, &Community_limit_trials);
    sprintf(param_str, "community_limit[%d]", s);
    get_param((char *) param_str, &Community_limit[s]);
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


void Community::reset() {
  int strains = population->get_strains();
  for (int s = 0; s < strains; s++) {
    susceptibles[s].clear();
    for (int group = 0; group < groups; group++) {
      infectious_list[s][group].clear();
    }
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


double Community::get_transmission_prob(int strain, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(strain, i);
  int col = get_group(strain, s);
  double tr_pr = Community_contact_prob[strain][row][col];
  return tr_pr;
}

int Community::get_group(int strain, Person * per) {
  int age = per->get_age();
  if (age < 18) { return 0; }
  else { return 1; }
}

double Community::get_contacts_per_day(int strain) {
  return Community_contacts_per_day[strain];
}

void Community::add_susceptible(int strain, Person * per) {
  susceptibles[strain].push_back(per);
  S[strain]++;
  assert (S[strain] == static_cast <int> (susceptibles[strain].size()));
}


void Community::delete_susceptible(int strain, Person * per) {
}

void Community::add_infectious(int strain, Person * per) {
  int group = get_group(strain, per);
  infectious_list[strain][group].insert(per);
  I[strain]++;
  assert (I[strain] == (static_cast <int> (infectious_list[strain][0].size()) +
			static_cast <int> (infectious_list[strain][1].size())));
  if (per->get_strain_status(strain) == 'I') {
    Sympt[strain]++;
    cases[strain]++;
    total_cases[strain]++;
  }
}

void Community::delete_infectious(int strain, Person * per) {
  int group = get_group(strain, per);
  assert(infectious_list[strain][group].find(per) != infectious_list[strain][group].end());
  infectious_list[strain][group].erase(per);
  I[strain]--;
  if (per->get_strain_status(strain)=='I') {
    Sympt[strain]--;
  }
  assert (I[strain] == (static_cast <int> (infectious_list[strain][0].size()) +
			static_cast <int> (infectious_list[strain][1].size())));
}

void Community::spread_infection(int day, int s) {
  if (Verbose > 1) { print(s); }
  // if (N < 2) return;
	
  set<Person *>::iterator itr;
  Strain * strain = population->get_strain(s);
  double beta = strain->get_transmissibility();

  // expected number of susceptible contacts for each infectious person
  double contacts = get_contacts_per_day(s);

  // expected upper bound on number of contacts resulting in infection (per infector)
  contacts *= beta;

  if (Verbose > 1) {
    printf("Strain %d, expected suscept contacts = %.3f\n",
           s, get_contacts_per_day(s));
    printf("beta = %.10f\n", beta);
    printf("effective contacts = %f\n", contacts);
    fflush(stdout);
  }
	
  for (int source_group = 0; source_group < groups; source_group++) {
    for (itr = infectious_list[s][source_group].begin();
	 itr != infectious_list[s][source_group].end(); itr++) {
      if (S[s] == 0) break;
      Person * infector = *itr;			// infectious indiv
      assert(infector->get_strain_status(s) == 'I' ||
	     infector->get_strain_status(s) == 'i');
		
      // skip if this infector did not visit today
      if (Verbose > 1) { printf("Is infector %d here?  ", infector->get_id()); }
      if (infector->is_on_schedule(day, id, type) == false) {  
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
		
      // stochastically round off the expected value of the contact counts
      int contact_count = (int) my_contacts;
      double r = RANDOM();
      if (r < my_contacts - contact_count) contact_count++;
      if (Verbose > 1) {
	printf("my contact_count = %d  r = %f\n", contact_count, r);
	fflush(stdout);
      }
		
      Place *h1 = NULL;
      double lat1 = 0.0;
      double lon1 = 0.0;
      if (0 <= Community_limit[s]) {
	h1 = infector->get_household();
	lat1 = h1->get_latitude();
	lon1 = h1->get_longitude();
      }

      for (int contact = 0; contact < contact_count; contact++) {
	Person * infectee = get_possible_infectee(s, infector, lat1, lon1);

	// check probability of infection, before we update schedule
	int dest_group = get_group(s, infectee);
	double transmission_prob = Community_contact_prob[s][source_group][dest_group];
	if (Verbose > 1) {
	  printf("transmission_prob = %f\n", transmission_prob);
	}
	r = RANDOM();
	if (r < transmission_prob) {
	  if (Verbose > 1) {
	    printf("r = %f < transmission_prob = %f\n", r, transmission_prob);
	  }

	  // is the victim here today, and still susceptible?
	  if (infectee->is_on_schedule(day, id, type) && infectee->get_strain_status(s) == 'S') {
	    if (Verbose > 1) { printf("Victim is here\n"); }
    
	    // get the victim's susceptibility
	    double susceptibility = infectee->get_susceptibility(s);
	    if (Verbose > 1) {
	      printf("susceptibility = %f\n", susceptibility);
	    }
    
	    // attempt transmission
	    double r = RANDOM();
	    if (r < susceptibility) {
	      // transmission succeeded
	      Infection * infection = new Infection(strain, infector, infectee, this, day);
	      infectee->become_exposed(infection);
	      infector->add_infectee(s);
	      if (Verbose > 1) {
		printf("transmission succeeded: r = %f\n", r);
		if (infector->get_exposure_date(s) == 0) {
		  printf("SEED infection day %i from %d to %d\n",
			 day, infector->get_id(),infectee->get_id());
		} else {
		  printf("infection day %i of strain %i from %d to %d\n",
			 day, s, infector->get_id(),infectee->get_id());
		}
	      }
	    }
	  }
	}
	else {
	  if (Verbose > 1) { printf("transmission failed: r = %f\n", r); }
	}
      } // end contact loop
    } // end infectious list loop
  }
  if (Verbose > 1) { fflush(stdout); }
}

Person * Community::get_possible_infectee(int strain, Person * infector, double lat1, double lon1) {
  Person * infectee = NULL;
  Person * candidate;
  double r;
  int pos;

  if (Community_limit[strain] < 0) {
    // select a random target
    r = RANDOM();
    pos = (int) (r*S[strain]);
    infectee = susceptibles[strain][pos];
  }
  else {
    // select a random target using a gravity model
    int trial = 0;
    int not_found = 1;
    double dist;
    double min_dist = -1;
    // take at most Community_limit_trials
    while (not_found && (trial < Community_limit_trials)) {
      trial++;
      pos = (int) (r*S[strain]);
      candidate = susceptibles[strain][pos];
      Place *h2 = candidate->get_household();
      double lat2 = h2->get_latitude();
      double lon2 = h2->get_longitude();
      
      dist = geo_dist(lat1, lat2, lon1, lon2, 0);
      if (min_dist < 0 || dist < min_dist) {
	infectee = candidate;
	min_dist = dist;
      }
      r = RANDOM();
      if (dist == 0.0 ||
	  (dist < Community_limit[strain] && r < 25.0/(dist*dist))) {
	infectee = candidate;
	not_found = 0;
      }
    }
    // if not found, then infectee points to the closest candidate
    if (Verbose > 2) {
      printf("Gravity: trial = %d,  max_trials = %d  ", trial, Community_limit_trials);
      printf("dist = %f,  min_dist = %f  r = %f\n", dist, min_dist, r);
      fflush(stdout);
    }
  }
  if (Verbose > 1) {
    if (infectee != NULL) {
      printf("possible victim = %d  prof = %d pos = %d  S[%d] = %d\n",
	     infectee->get_id(), infectee->get_behavior()->get_profile(),
	     pos, strain, S[strain]);
    }
    else {
      printf("possible victim = None found\n");
    }
    fflush(stdout);
  }
  return(infectee);
}

double Community::geo_dist(double lat1, double lat2, double lon1, double lon2, int ic) {
  double z;
  double x,y,v;
  if(ic == 1) {
    x=(lat1-lat2)/3.28/5280;
    y=(lon1-lon2)/3.28/5280;
    v= x*x + y*y;
    z=v;
    z=sqrt(z);
  } else {
    x = lat1/57.2958;
    y = lat2/57.2958;
    v = (lon2-lon1)/57.2958;
    z=3963.0* acos(sin(x)*sin(y)+cos(x)*cos(y)*cos(v));
  }
  return z;
}

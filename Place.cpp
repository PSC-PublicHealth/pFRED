//
//
// File: Place.cpp
//

#include "Place.hpp"

extern Person *Pop;
extern Disease *Dis;
extern int Diseases;
extern int Verbose;

Place::Place() {
  // printf("new place with diseases = %d\n", Diseases); fflush(stdout);

  beta = (double *) malloc(Diseases*sizeof(double));
  if (beta == NULL) {
    printf("Help! beta allocation failure\n");
    abort();
  }

  contact_prob = (double *) malloc(Diseases*sizeof(double));
  if (contact_prob == NULL) {
    printf("Help! contact_prob allocation failure\n");
    abort();
  }

  contacts_per_day = (int *) malloc(Diseases*sizeof(int));
  if (contacts_per_day == NULL) {
    printf("Help! contacts_per_day allocation failure\n");
    abort();
  }

  susceptibles = (vector <int> *) malloc(Diseases*sizeof(vector <int>));
  if (susceptibles == NULL) {
    printf("Help! susceptibles allocation failure\n");
    abort();
  }

  infectious = (vector <int> *) malloc(Diseases*sizeof(vector <int>));
  if (infectious == NULL) {
    printf("Help! infectious allocation failure\n");
    abort();
  }

  S = (int *) malloc(Diseases*sizeof(int));
  if (S == NULL) {
    printf("Help! S allocation failure\n");
    abort();
  }

  I = (int *) malloc(Diseases*sizeof(int));
  if (I == NULL) {
    printf("Help! I allocation failure\n");
    abort();
  }

  Sympt = (int *) malloc(Diseases*sizeof(int));
  if (Sympt == NULL) {
    printf("Help! Sympt allocation failure\n");
    abort();
  }

  for (int d = 0; d < Diseases; d++) {
    beta[d] = Dis[d].get_transmissibility();
  }
}
  
void Place::setup(int loc, char *lab, double lon, double lat, int diseases) {
  id = loc;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;
  type = 'U';
  reset();
}



void Place::reset() {
  N = 0;
  for (int d = 0; d < Diseases; d++) {
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
  printf("Place %d label %s type %c ", id, label, type);
  printf("contacts %d contact_p %f S %d I %d N %d\n",
	 contacts_per_day[dis], contact_prob[dis], S[dis], I[dis], N);
  fflush(stdout);
}

void Place::add_susceptible(int dis, int per) {
  if (Verbose > 2) {
    printf("add suscepteptible dis %d per %d place id %d\n",dis,per,id);
    fflush(stdout);
  }
  // print(0);
  // printf("sus size = %d\n", (int)susceptibles[dis].size()); fflush(stdout);
  // susceptibles[dis].insert(susceptibles[dis].end(), per);
  try
    {
      // printf("start trying\n"); fflush(stdout);
      susceptibles[dis].push_back(per);
      // printf("done trying\n"); fflush(stdout);
    }
  catch (exception& e)
    {
      cout << "add_susceptible: exception caught: " << e.what() << endl;
    }
  
  // printf("sus size = %d\n", (int)susceptibles[dis].size()); fflush(stdout);
  // print(0);

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
  // infectious[dis].insert(infectious[dis].end(), per);
  infectious[dis].push_back(per);
  I[dis]++;
  if (Pop[per].get_disease_status(dis) == 'I') {
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
  if (Pop[per].get_disease_status(dis)=='I') {
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

  for (int d = 0; d < Diseases; d++) {
    if (Verbose > 1) { print(d); }
    if (N < 2) return;
    if (S[d] == 0) continue;

    // expected number of susceptible contacts for each infectious person
    double contacts = contacts_per_day[d] * ((double) S[d]) / ((double) (N-1));
    if (Verbose > 1) {
      printf("expected suscept contacts = %f\n", contacts);
      fflush(stdout);
    }
    
    // expected u.b. on number of contacts resulting in infection (per infective)
    contacts *= beta[d] * contact_prob[d];
    if (Verbose > 1) {
      printf("beta = %f\n", beta[d]);
      printf("contact prob = %f\n", contact_prob[d]);
      printf("effective contacts = %f\n", contacts);
      fflush(stdout);
    }

    for (itr = infectious[d].begin(); itr != infectious[d].end(); itr++) {
      int i = *itr;				// infectious indiv

      if (!Pop[i].on_schedule(day, id)) continue;
      if (Verbose > 1) { printf("infected = %d\n", i); }

      // expected number of infective contact events
      double my_contacts = contacts * Pop[i].get_infectivity(d);
      if (Verbose > 1) {
	printf("my effective contacts = %f\n", my_contacts);
	fflush(stdout);
      }

      // get count of contacts resulting in infection
      // int contact_count = draw_poisson(my_contacts);
      // use the (randomly rounded) expected value:
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
	if (Pop[s].on_schedule(day,id) && Pop[s].get_disease_status(d) == 'S') {
	  if (Verbose > 1) {
	    printf("susceptibility = %f\n", Pop[s].get_susceptibility(d));
	    fflush(stdout);
	  }
	  if (RANDOM() < Pop[s].get_susceptibility(d)) {
	    if (Verbose > 1) { printf("infection!\n"); }
	    Pop[s].make_exposed(d, i, id, day);
	    Pop[i].add_infectee(d);
	  }
	  else {
	    if (Verbose > 1) { printf("no infection\n"); }
	  }
	}
      } // end contact loop
    } // end infectious list loop
  }  // end disease loop
}

int Place::is_open(int day) {
  return (day < close_date || open_date <= day);
}

int Place::should_be_open(int day, int dis) {
  return 1;
}


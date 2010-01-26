/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Spread.cc
//

#include "Spread.h"
#include "Strain.h"

#include <stdio.h>
#include <new>
#include <iostream>
using namespace std;
#include "Random.h"
#include "Params.h"
#include "Person.h"
#include "Global.h"
#include "Infection.h"
#include "Locations.h"
#include "Place.h"

Spread::Spread(Strain *str) {
  char filename[80];
  char param_name[80];

  FILE *fp;
  int day, new_cases;

  strain = str;

  sprintf(param_name, "new_cases_file[%d]", strain->get_id());

  //First see if the new_cases_file parameter exists
  if(does_param_exist(param_name)) {
    get_param(param_name, filename);

    fp = fopen(filename, "r");
    if (fp != NULL) {
      while (fscanf(fp, "%i %i", &day, &new_cases) == 2) {
        new_cases_map.insert(pair<int, int>(day, new_cases));
      }

      fclose(fp);
    } else {
      printf("Help!  Can't read new_cases_file %s\n", filename);
      abort();
    }

  } else {
    //Just use the old parameter index_cases[%d] to create the map
    int temp_indx_cases;

    sprintf(param_name, "index_cases[%d]", strain->get_id());
    get_param(param_name, &temp_indx_cases);

    //insert the index_cases in the 0 position in the map
    new_cases_map.insert(pair<int, int>(0, temp_indx_cases));
    //insert 0 in the 1 position in the map to indicate that no more
    //new cases should be added
    new_cases_map.insert(pair<int, int>(1, 0));
  }

  map<int,int>::iterator new_cases_map_itr;

  //See if there are index cases (i.e. people exposed on day 0)
  new_cases_map_itr = new_cases_map.find(0);
  if(new_cases_map_itr != new_cases_map.end()) {
	  new_cases = index_cases = new_cases_map_itr -> second;
  }
  else {
	  new_cases = index_cases = 0;
  }
}

void Spread::reset() {
  exposed.clear();
  infectious.clear();
  not_yet_exposed.clear();
  attack_rate = 0.0;
  S = E = I = R = 0;
}

void Spread::start_outbreak(Person *pop, int pop_size) {

  for(int i = 0; i < pop_size; i++) {
	  not_yet_exposed.push_back(&pop[i]);
  }

  // shuffle the not_yet_exposed vector
  for(int i = 0; i < pop_size; i++) {
	  int n = IRAND(i, pop_size - 1);

	  //switch current value with the random one
	  Person * hold = not_yet_exposed[i];
	  not_yet_exposed[i] = not_yet_exposed[n];
	  not_yet_exposed[n] = hold;
  }
}

void Spread::update_stats(Person *pop, int pop_size, int day) {
  S = E = I = R = 0;
  for (int p = 0; p < pop_size; p++) {
    char status = pop[p].get_strain_status(strain->get_id());
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
	  day, strain->get_id(), S, E, I, R, N, attack_rate);
  fflush(Outfp);
  
  if (Verbose) {
    fprintf(Statusfp,
	    "Day %3d  Str %d  S %7d  E %7d  I %7d  R %7d  N %7d  AR %5.2f\n\n",
	    day, strain->get_id(), S, E, I, R, N, attack_rate);
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

  /*
   * Check to see if there are any new cases that we are simply force adding
   */
  // See if there is a change in the number of new cases from the new_cases_map
  map<int,int>::iterator new_cases_map_itr;
  new_cases_map_itr = new_cases_map.find(day);
  if(new_cases_map_itr != new_cases_map.end()) {
	  new_cases = new_cases_map_itr -> second;
  }

  // Force expose the new cases as long as there are still people susceptible
  int i = 0;
  while(i < new_cases && !not_yet_exposed.empty())
  {
	bool exposed = false;
	while(!exposed && !not_yet_exposed.empty())
    {
      Person * temp = not_yet_exposed.back();

	  if((temp -> get_strain_status(strain -> get_id())) == 'S')
	  {
	    // New infections are a special case - we don't initialize using
	    // Strain::attempt_infection() meaning we don't roll the dice to see if
	    // we'll expose the person.  We just do it.
		Person * temp = not_yet_exposed.back();
	    Infection * infection = new Infection(strain, NULL, temp, NULL, day);
	    temp -> become_exposed(infection);
	    exposed = true;
	  }

	  not_yet_exposed.pop_back();
	}

	i++;
  }

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
      if (place->is_open(day) && place->should_be_open(day, strain->get_id())) {
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

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Person.cpp
//

#include "Person.hpp";
#include "Global.hpp"
#include "Profile.hpp"
#include "Population.hpp"
#include "Strain.hpp"
#include "Random.hpp"
#include "Place.hpp"
#include "Locations.hpp"

void Person::setup(int i, int a, char g, int m, int o, int p, int h,
		   int n, int s, int c, int w, int off, int pro) 
{
  id = i;
  age = a;
  sex = g;
  occupation = 'U';
  marrital_status = m;
  profession = p;
  favorite_place[0] = h;
  favorite_place[1] = n;
  favorite_place[2] = s;
  favorite_place[3] = c;
  favorite_place[4] = w;
  favorite_place[5] = off;
  favorite_places = 6;
  profile = pro;

  schedule_updated = -1;
  scheduled_places = 0;

  int strains = Strain::get_strains();
  strain_status = new (nothrow) char [strains];
  if (strain_status == NULL) {
    printf("Help! strain_status allocation failure for Person %d\n", id);
    abort();
  }

  latent_period = new (nothrow) int [strains];
  if (latent_period == NULL) {
    printf("Help! latent_period allocation failure for Person %d\n", id);
    abort();
  }

  infectious_period = new (nothrow) int [strains];
  if (infectious_period == NULL) {
    printf("Help! infectious_period allocation failure for Person %d\n", id);
    abort();
  }

  exposure_date = new (nothrow) int [strains];
  if (exposure_date == NULL) {
    printf("Help! exposure_date allocation failure for Person %d\n", id);
    abort();
  }

  infectious_date = new (nothrow) int [strains];
  if (infectious_date == NULL) {
    printf("Help! infectious_date allocation failure for Person %d\n", id);
    abort();
  }

  recovered_date = new (nothrow) int [strains];
  if (recovered_date == NULL) {
    printf("Help! recovered_date allocation failure for Person %d\n", id);
    abort();
  }

  infector = new (nothrow) int [strains];
  if (infector == NULL) {
    printf("Help! infector allocation failure for Person %d\n", id);
    abort();
  }

  infected_place = new (nothrow) int [strains];
  if (infected_place == NULL) {
    printf("Help! infected_place allocation failure for Person %d\n", id);
    abort();
  }

  infected_place_type = new (nothrow) char [strains];
  if (infected_place_type == NULL) {
    printf("Help! infected_place_type allocation failure for Person %d\n", id);
    abort();
  }

  infectees = new (nothrow) int [strains];
  if (infectees == NULL) {
    printf("Help! infectees allocation failure for Person %d\n", id);
    abort();
  }

  susceptibility = new (nothrow) double [strains];
  if (susceptibility == NULL) {
    printf("Help! susceptibility allocation failure for Person %d\n", id);
    abort();
  }

  infectivity = new (nothrow) double [strains];
  if (infectivity == NULL) {
    printf("Help! infectivity allocation failure for Person %d\n", id);
    abort();
  }

  role = new (nothrow) char [strains];
  if (role == NULL) {
    printf("Help! role allocation failure for Person %d\n", id);
    abort();
  }

  for (int s = 0; s < strains; s++) {
    infected_place[s] = -1;
    infected_place_type[s] = 'X';
    role[s] = NO_ROLE;
  }
}
  
void Person::print(int d) {
  fprintf(Tracefp, "%c id %7d  a %3d  s %c %c ",
	  strain_status[d], id, age, sex, occupation);
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
	  exposure_date[d], infectious_date[d], recovered_date[d]);
  fprintf(Tracefp, "places %d ", favorite_places);
  fprintf(Tracefp, "infected_at %c %6d ",
	  infected_place_type[d], infected_place[d]);
  fprintf(Tracefp, "infector %d ", infector[d]);
  fprintf(Tracefp, "infectees %d\n", infectees[d]);
  fflush(Tracefp);
}

void Person::print_out(int d) {
  printf("%c id %7d  a %3d  s %c %c ",
	 strain_status[d], id, age, sex, occupation);
  printf("exp: %2d  inf: %2d  rem: %2d ",
	 exposure_date[d], infectious_date[d], recovered_date[d]);
  printf("places %d ", favorite_places);
  printf("infected_at %c %6d ",
	 infected_place_type[d], infected_place[d]);
  printf("infector %d ", infector[d]);
  printf("infectees %d\n", infectees[d]);
  fflush(stdout);
}


void Person::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d\n", id);
  for (int j = 0; j < scheduled_places; j++) {
    fprintf(Statusfp, "%d ", schedule[j]);
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}
  
void Person::make_susceptible() {
  if (Verbose > 2) { fprintf(Statusfp, "SUSCEPTIBLE person %d\n", id); }
  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    strain_status[s] = 'S';
    exposure_date[s] = infectious_date[s] = recovered_date[s] = -1;
    infected_place[s] = -1;
    infected_place_type[s] = 'X';
    infector[s] = -1;
    infectees[s] = 0;
    susceptibility[s] = 1.0;
    infectivity[s] = 0.0;
    role[s] = NO_ROLE;

    for (int p = 0; p < favorite_places; p++) {
      if (favorite_place[p] == -1) continue;
      Loc.add_susceptible_to_place(favorite_place[p], s, id);
    }
  }
  schedule_updated = -1;
  scheduled_places = 0;
}

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

void Person::make_exposed(int strain, int per, int place, char type, int day) {
  if (Verbose > 1) { fprintf(Statusfp, "EXPOSED person %d\n", id); }
  strain_status[strain] = 'E';
  exposure_date[strain] = day;
  latent_period[strain] = Strain::get_days_latent(strain);
  infectious_period[strain] = Strain::get_days_infectious(strain);
  infectious_date[strain] = exposure_date[strain] + latent_period[strain];
  recovered_date[strain] = infectious_date[strain] + infectious_period[strain];
  infector[strain] = per;
  infected_place[strain] = place;
  if (place == -1) { 
    infected_place_type[strain] = 'X';
  }
  else {
    infected_place_type[strain] = type;
  }
  susceptibility[strain] = 0.0;
  Pop.insert_into_exposed_list(strain, id);
  if (Verbose > 2) { print_out(strain); }
}
  
void Person::make_infectious(int d) {
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for strain %d\n", id, d);
    fflush(Statusfp);
  }
  if (RANDOM() < Strain::get_prob_symptomatic(d)) {
    strain_status[d] = 'I';
    infectivity[d] = 1.0;
  }
  else {
    strain_status[d] = 'i';
    infectivity[d] = 0.5;
  }
  Pop.remove_from_exposed_list(d, id);
  Pop.insert_into_infectious_list(d, id);
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == -1) continue;
    Loc.delete_susceptible_from_place(favorite_place[p], d, id);
    if (Test == 0 || exposure_date[d] == 0) {
	Loc.add_infectious_to_place(favorite_place[p], d, id);
    }
  }
}

void Person::make_recovered(int strain) {
  if (Verbose > 2) {
    fprintf(Statusfp, "RECOVERED person %d for strain %d\n", id, strain);
    print_out(strain);
    fflush(Statusfp);
  }
  strain_status[strain] = 'R';
  Pop.remove_from_infectious_list(strain, id);
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == -1) continue;
    if (Test == 0 || exposure_date[strain] == 0)
      Loc.delete_infectious_from_place(favorite_place[p], strain, id);
  }
  // print recovered agents into Trace file
  print(strain);
}

int Person::is_on_schedule(int day, int loc) {
  int p = 0;
  if (schedule_updated < day) 
    update_schedule(day);
  while (p < scheduled_places && schedule[p] != loc) p++;
  return (p < scheduled_places);
}

void Person::update_schedule(int day) {
  int day_of_week;
  if (schedule_updated < day) {
    schedule_updated = day;
    scheduled_places = 0;
    for (int p = 0; p < favorite_places; p++) {
      on_schedule[p] = 0;
    }
    day_of_week = (day+Start_day) % DAYS_PER_WEEK;

    // probably stay at home if symptomatic
    if (is_symptomatic()) {
      if (RANDOM() < Strain::get_prob_stay_home()) {
	scheduled_places = 1;
	schedule[0] = favorite_place[0];
	on_schedule[0] = 1;
      }
    }

#if NOT_IMPLEMENTED
    // if not staying home, decide if traveling
    if (scheduled_places == 0) {
      if (is_traveling(profile, day_of_week)) {
	// pick a favorite destination
	int dest = IRAND(0, favorite_destinations);
      }
    }
#endif

    // if not staying home or traveling, consult usual schedule
    if (scheduled_places == 0) {
      for (int p = 0; p < favorite_places; p++) {
	// visit classroom or office iff going to school or work
	if (p == 3 || p == 5) {
	  if (on_schedule[p-1]) {
	    on_schedule[p] = 1;
	    schedule[scheduled_places++] = favorite_place[p];
	  }
	}
	else if (favorite_place[p] != -1 &&
		 is_visited(p, profile, day_of_week)) {
	  on_schedule[p] = 1;
	  schedule[scheduled_places++] = favorite_place[p];
	}
      }
    }

    if (Verbose > 2) {
      printf("update_schedule on day %d\n", day);
      print_out(0);
      print_schedule();
      fflush(stdout);
    }
  }
}

void Person::get_schedule(int *n, int *sched) {
  *n = scheduled_places;
  for (int i = 0; i < scheduled_places; i++)
    sched[i] = schedule[i];
}

void Person::set_occupation() {
  // set occupation by age
  if (age < 5) { occupation = 'C'; }
  else if (age < 19) { occupation = 'S'; }
  else if (age < 65) { occupation = 'W'; }
  else { occupation = 'R'; }

  // identify teachers
  for (int p = 0; p < favorite_places; p++) {
    if (favorite_place[p] == -1) continue;
    if (age > 18 && (Loc.get_type_of_place(favorite_place[p]) == SCHOOL))
      occupation = 'T';
  }
}

int Person::is_symptomatic() {
  int strains = Strain::get_strains();
  for (int s = 0; s < strains; s++) {
    if (strain_status[s] == 'I')
      return 1;
  }
  return 0;
}

void Person::behave(int day) {}

  // Health Behavior Model
void Person::update_perceived_susceptibility() {};
void Person::update_perceived_severity() {};
void Person::update_perceived_barrier() {};
void Person::update_perceived_benefit() {};


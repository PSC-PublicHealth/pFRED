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

extern vector<Place*> Loc;
extern Disease *Dis;
extern int Diseases;
extern FILE *Statusfp;
extern FILE *Tracefp;
extern int Test;
extern int Verbose;

void Person::setup(int i, int a, char g) {
  id = i;
  age = a;
  sex = g;
  occupation = 'U';

  disease_status = new (nothrow) char [Diseases];
  if (disease_status == NULL) {
    printf("Help! disease_status allocation failure\n");
    abort();
  }

  latent_period = new (nothrow) int [Diseases];
  if (latent_period == NULL) {
    printf("Help! latent_period allocation failure\n");
    abort();
  }

  infectious_period = new (nothrow) int [Diseases];
  if (infectious_period == NULL) {
    printf("Help! infectious_period allocation failure\n");
    abort();
  }

  exposure_date = new (nothrow) int [Diseases];
  if (exposure_date == NULL) {
    printf("Help! exposure_date allocation failure\n");
    abort();
  }

  infectious_date = new (nothrow) int [Diseases];
  if (infectious_date == NULL) {
    printf("Help! infectious_date allocation failure\n");
    abort();
  }

  recovered_date = new (nothrow) int [Diseases];
  if (recovered_date == NULL) {
    printf("Help! recovered_date allocation failure\n");
    abort();
  }

  infector = new (nothrow) int [Diseases];
  if (infector == NULL) {
    printf("Help! infector allocation failure\n");
    abort();
  }

  infected_place = new (nothrow) int [Diseases];
  if (infected_place == NULL) {
    printf("Help! infected_place allocation failure\n");
    abort();
  }

  infected_place_type = new (nothrow) char [Diseases];
  if (infected_place_type == NULL) {
    printf("Help! infected_place_type allocation failure\n");
    abort();
  }

  infectees = new (nothrow) int [Diseases];
  if (infectees == NULL) {
    printf("Help! infectees allocation failure\n");
    abort();
  }

  susceptibility = new (nothrow) double [Diseases];
  if (susceptibility == NULL) {
    printf("Help! susceptibility allocation failure\n");
    abort();
  }

  infectivity = new (nothrow) double [Diseases];
  if (infectivity == NULL) {
    printf("Help! infectivity allocation failure\n");
    abort();
  }

  role = new (nothrow) char [Diseases];
  if (role == NULL) {
    printf("Help! role allocation failure\n");
    abort();
  }

  for (int d = 0; d < Diseases; d++) {
    infected_place[d] = -1;
    infected_place_type[d] = 'X';
    role[d] = NO_ROLE;
  }
}
  
void Person::print(int d) {
  fprintf(Tracefp, "%c id %7d  a %3d  s %c %c ",
	  disease_status[d], id, age, sex, occupation);
  fprintf(Tracefp, "exp: %2d  inf: %2d  rem: %2d ",
	 exposure_date[d], infectious_date[d], recovered_date[d]);
  fprintf(Tracefp, "places %d ", places);
  fprintf(Tracefp, "infected_at %c %6d ",
	  infected_place_type[d], infected_place[d]);
  fprintf(Tracefp, "infector %d ", infector[d]);
  fprintf(Tracefp, "infectees %d\n", infectees[d]);
  fflush(Tracefp);
}

void Person::print_out(int d) {
  printf("%c id %7d  a %3d  s %c %c ",
	  disease_status[d], id, age, sex, occupation);
  printf("exp: %2d  inf: %2d  rem: %2d ",
	 exposure_date[d], infectious_date[d], recovered_date[d]);
  printf("places %d ", places);
  printf("infected_at %c %6d ",
	  infected_place_type[d], infected_place[d]);
  printf("infector %d ", infector[d]);
  printf("infectees %d\n", infectees[d]);
  fflush(stdout);
}

void Person::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d\n", id);
  for (int day = 0; day < DAYS_PER_WEEK; day++) {
    fprintf(Statusfp, "Day %d: ", day); 
    for (int j = 0; j < (int) schedule[day].size(); j++) {
      fprintf(Statusfp, "%d ", schedule[day][j]);
    }
    fprintf(Statusfp, "\n");
  }
}

void Person::make_susceptible() {
  if (Verbose > 2) { fprintf(Statusfp, "SUSCEPTIBLE person %d\n", id); }
  for (int d = 0; d < Diseases; d++) {
    disease_status[d] = 'S';
    exposure_date[d] = infectious_date[d] = recovered_date[d] = -1;
    infected_place[d] = -1;
    infected_place_type[d] = 'X';
    infector[d] = -1;
    infectees[d] = 0;
    susceptibility[d] = 1.0;
    infectivity[d] = 0.0;

    for (int p = 0; p < places; p++) {
      place[p]->add_susceptible(d, id);
      if (Verbose > 2) {
	fprintf(Statusfp, "place %d S %d\n",
		place[p]->get_id(), place[p]->get_S(d));
	fflush(Statusfp);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////
//
// Determines the transition dates for this person.
//
///////////////////////////////////////////////////////////////////////

void Person::make_exposed(int dis, int person_id, int place_id, int day) {
  if (Verbose > 1) { fprintf(Statusfp, "EXPOSED person %d\n", id); }
  disease_status[dis] = 'E';
  exposure_date[dis] = day;
  latent_period[dis] = Dis[dis].get_days_latent();
  infectious_period[dis] = Dis[dis].get_days_infectious();
  infectious_date[dis] = exposure_date[dis] + latent_period[dis];
  recovered_date[dis] = infectious_date[dis] + infectious_period[dis];
  infector[dis] = person_id;
  infected_place[dis] = place_id;
  if (place_id == -1) { 
    infected_place_type[dis] = 'X';
  }
  else {
    infected_place_type[dis] = Loc[place_id]->get_type();
  }
  susceptibility[dis] = 0.0;
  insert_into_exposed_list(dis, id);
  if (Verbose > 1) { print_out(dis); }
}
  
void Person::make_infectious(int d) {
  if (Verbose > 2) {
    fprintf(Statusfp, "INFECTIOUS person %d for disease %d\n", id, d); }
  if (RANDOM() < Dis[d].get_prob_symptomatic()) {
    disease_status[d] = 'I';
    infectivity[d] = 1.0;
  }
  else {
    disease_status[d] = 'i';
    infectivity[d] = 0.5;
  }
  remove_from_exposed_list(d, id);
  insert_into_infectious_list(d, id);
  for (int p = 0; p < places; p++) {
    place[p]->delete_susceptible(d, id);
    if (Verbose > 2) {
      fprintf(Statusfp, "place %d S %d\n", place[p]->get_id(), place[p]->get_S(d));
      fflush(Statusfp);
    }
    if (Test == 0 || exposure_date[d] == 0)
      place[p]->add_infectious(d, id);
    if (Verbose > 2) {
      fprintf(Statusfp, "place %d I %d\n", place[p]->get_id(), place[p]->get_I(d));
      fflush(Statusfp);
    }
  }
}
  
void Person::make_recovered(int dis) {
  if (Verbose > 2) {
    fprintf(Statusfp, "REMOVED person %d for disease %d\n", id, dis);
  }
  disease_status[dis] = 'R';
  remove_from_infectious_list(dis, id);
  for (int p = 0; p < places; p++) {
    if (Test == 0 || exposure_date[dis] == 0)
      place[p]->delete_infectious(dis, id);
    if (Verbose > 2) {
      fprintf(Statusfp, "place %d I %d\n", place[p]->get_id(), place[p]->get_I(dis));
      fflush(Statusfp);
    }
  }
  // print recovered agents into Trace file
  print(dis);
}

int Person::on_schedule(int day, int pl) {
  extern int Start_day;
  day = (day + Start_day) % DAYS_PER_WEEK;
  for (int p = 0; p < places; p++) {
    if (schedule[day][p] == pl) {
      return (RANDOM() < visit_prob[day][p]);
    }
  }
  return 0;
}

void Person::add_to_schedule(int day, int loc, Place *pl, double prob) {
  day = day % DAYS_PER_WEEK;
  schedule[day].push_back(loc);
  visit_prob[day].push_back(prob);

  // add place list if needed
  int found = 0;
  for (int p = 0; p < places; p++) {
    if (place[p]->get_id() == loc) { found = 1; break; }
  }
  if (!found) {
    place.push_back(pl);
    places++;
  }
}

void Person::set_occupation() {
  // set occupation by age
  if (age < 5) { occupation = 'C'; }
  else if (age < 19) { occupation = 'S'; }
  else if (age < 65) { occupation = 'W'; }
  else { occupation = 'R'; }

  // identify teachers
  for (int p = 0; p < places; p++) {
    if (age > 18 && place[p]->get_type() == 'S')
      occupation = 'T';
  }
}



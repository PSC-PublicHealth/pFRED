/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Loc.cpp
//

#include "Loc.hpp"

extern set <int> *Infectious;

char Locfile[80];
Place ** Loc;
int Locations;

void get_location_parameters() {
}


void setup_locations() {
  FILE *fp;

  if (Verbose) {
    fprintf(Statusfp, "setup locations entered\n"); fflush(Statusfp);
  }

  get_param((char *) "locfile", Locfile);

  fp = fopen(Locfile, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "Locfile %s not found\n", Locfile);
    abort();
  }

  fscanf(fp, "Locations = %d", &Locations);
  if (Verbose) {
    fprintf(Statusfp, "Locations = %d\n", Locations); fflush(Statusfp);
  }

  Loc = new (nothrow) Place * [Locations];
  if (Loc == NULL) { printf("Help! Loc allocation failure\n"); abort(); }

  for (int loc = 0; loc < Locations; loc++) {
    int id;
    char s[32];
    char loctype;
    double lon, lat;
    int container;

    // fprintf(Statusfp, "reading location %d\n", loc); fflush(Statusfp);

    if (fscanf(fp, "%d %s %c %lf %lf %d",
	       &id, s, &loctype, &lat, &lon, &container) != 6) {
      fprintf(Statusfp, "Help! Read failure for location %d\n", loc);
      abort();
    }
    if (id != loc) {
      fprintf(Statusfp, "Help! Read index %d for location %d\n", id, loc);
      abort();
    }

    // printf("loctype = %c\n", loctype); fflush(stdout);
    Place *place;
    if (loctype == HOUSEHOLD) {
      place = new (nothrow) Household(id, s, lon, lat, container);
    }
    else if (loctype == NEIGHBORHOOD) {
      place = new (nothrow) Neighborhood(id, s, lon, lat, container);
    }
    else if (loctype == SCHOOL) {
      place = new (nothrow) School(id, s, lon, lat, container);
    }
    else if (loctype == CLASSROOM) {
      place = new (nothrow) Classroom(id, s, lon, lat, container);
    }
    else if (loctype == WORKPLACE) {
      place = new (nothrow) Workplace(id, s, lon, lat, container);
    }
    else if (loctype == OFFICE) {
      place = new (nothrow) Office(id, s, lon, lat, container);
    }
    else if (loctype == HOSPITAL) {
      place = new (nothrow) Hospital(id, s, lon, lat, container);
    }
    else if (loctype == COMMUNITY) {
      place = new (nothrow) Community(id, s, lon, lat, container);
    }
    else {
      printf ("Help! bad loctype = %c\n", loctype);
      abort();
    }
    if (place == NULL) { printf("Help! allocation failure for loc %d\n", loc); abort(); }
    Loc[loc] = place;
  }
  fclose(fp);

  if (Verbose) {
    fprintf(Statusfp, "setup locations finished: Locations = %d\n", Locations);
    fflush(Statusfp);
  }
}

void reset_locations(int run) {

  if (Verbose > 2) {
    fprintf(Statusfp, "reset locations entered\n"); fflush(Statusfp);
  }

  for (int loc = 0; loc < Locations; loc++) {
    Loc[loc]->reset();
  }

  if (Verbose > 2) {
    fprintf(Statusfp, "reset locations finished\n"); fflush(Statusfp);
  }
}

void location_quality_control() {

  if (Verbose) {
    fprintf(Statusfp, "location quality control check\n"); fflush(Statusfp);
  }

  for (int loc = 0; loc < Locations; loc++) {
    if (Loc[loc]->get_size() < 1) {
      fprintf(Statusfp, "Help!  No one visits location %d\n", loc);
      Loc[loc]->print(0);
      continue;
    }

    if (Loc[loc]->get_size() == 1) {
      if (Verbose > 2) {
	fprintf(Statusfp, "Warning!  Only one visitor to location %d\n", loc);
	Loc[loc]->print(0);
      }
    }
  }

  if (Verbose) {
    int count[20];
    int total = 0;
    // size distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int loc = 0; loc < Locations; loc++) {
      if (Loc[loc]->get_type() == HOUSEHOLD) {
	int n = Loc[loc]->get_size();
	if (n < 15) { count[n]++; }
	else { count[14]++; }
	total++;
      }
    }
    fprintf(Statusfp, "\nHousehold size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    int count[20];
    int total = 0;
    // size distribution of schools
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int loc = 0; loc < Locations; loc++) {
      if (Loc[loc]->get_type() == SCHOOL) {
	int s = Loc[loc]->get_size();
	int n = s / 50;
	if (n < 20) { count[n]++; }
	else { count[19]++; }
	total++;
      }
    }
    fprintf(Statusfp, "\nSchool size distribution: %d schools\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     (c+1)*50, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    int count[50];
    int total = 0;
    // size distribution of classrooms
    for (int c = 0; c < 50; c++) { count[c] = 0; }
    for (int loc = 0; loc < Locations; loc++) {
      if (Loc[loc]->get_type() == CLASSROOM) {
	int s = Loc[loc]->get_size();
	int n = s;
	if (n < 50) { count[n]++; }
	else { count[50-1]++; }
	total++;
      }
    }
    fprintf(Statusfp, "\nClassroom size distribution: %d classrooms\n", total);
    for (int c = 0; c < 50; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    int count[20];
    int total = 0;
    // size distribution of workplaces
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int loc = 0; loc < Locations; loc++) {
      if (Loc[loc]->get_type() == WORKPLACE) {
	int s = Loc[loc]->get_size();
	int n = s / 50;
	if (n < 20) { count[n]++; }
	else { count[19]++; }
	total++;
      }
    }
    fprintf(Statusfp, "\nWorkplace size distribution: %d workplaces\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     (c+1)*50, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    int count[60];
    int total = 0;
    // size distribution of offices
    for (int c = 0; c < 60; c++) { count[c] = 0; }
    for (int loc = 0; loc < Locations; loc++) {
      if (Loc[loc]->get_type() == OFFICE) {
	int s = Loc[loc]->get_size();
	int n = s;
	if (n < 60) { count[n]++; }
	else { count[60-1]++; }
	total++;
      }
    }
    fprintf(Statusfp, "\nOffice size distribution: %d offices\n", total);
    for (int c = 0; c < 60; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
	     c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }

  if (Verbose) {
    fprintf(Statusfp, "location quality control finished\n"); fflush(Statusfp);
  }
}


void process_infectious_locations(int day) {
  set <int> places;
  set<int>::iterator itr;

  if (Verbose) {
    fprintf(Statusfp, "process infectious locations for day %d\n", day);
    fflush(Statusfp);
  }

  int diseases = get_diseases();
  for (int d = 0; d < diseases; d++) {

    if (Verbose > 3) {
      fprintf(Statusfp, "disease = %d  infectious = %d\n", d,
	      (int) (Infectious[d].size())); fflush(Statusfp);
    }

    // get list of infectious locations:
    for (itr = Infectious[d].begin(); itr != Infectious[d].end(); itr++) {
      int p = *itr;
      if (Verbose > 3) {
	fprintf(Statusfp, "day=%d p=%d\n", day, p);
	fflush(Statusfp);
      }
      int n;
      int schedule[100];
      update_schedule(p, day);
      get_schedule(p, &n, schedule);

      /*
      printf("size of schedule = %d\n", n); fflush(stdout);
      for (int j = 0; j < n; j++) {
	printf("schedule[%d] = %d\n", j, schedule[j]); fflush(stdout);
      }
      */

      for (int j = 0; j < n; j++) {
	int loc = schedule[j];
	if (Loc[loc]->is_open(day) && Loc[loc]->should_be_open(day, d)) {
	  places.insert(loc);
	}
      }
    }
  }

  if (Verbose) {
    fprintf(Statusfp, "Number of infectious places = %d\n", (int) places.size());
  }
  
  // infect visitors to infectious locations:
  for (itr = places.begin(); itr != places.end(); itr++ ) {
    int loc = *itr;
    if (Verbose > 1) {
      fprintf(Statusfp, "\nspread in location: %d\n", loc); fflush(Statusfp);
    }
    Loc[loc]->spread_infection(day);
  }

  if (Verbose) {
    fprintf(Statusfp, "process infectious locations for day %d complete\n", day);
    fflush(Statusfp);
  }

}

int get_open_status(int loc, int day) {
  return Loc[loc]->is_open(day);
}

void add_susceptible_to_place(int id, int dis, int per) {
  Loc[id]->add_susceptible(dis, per);
  if (Verbose > 2) {
    fprintf(Statusfp, "place %d S %d\n", id, Loc[id]->get_S(dis));
    fflush(Statusfp);
  }
}

void delete_susceptible_from_place(int id, int dis, int per) {
  Loc[id]->delete_susceptible(dis, per);
  if (Verbose > 2) {
    fprintf(Statusfp, "place %d S %d\n", id, Loc[id]->get_S(dis));
    fflush(Statusfp);
  }
}

void add_infectious_to_place(int id, int dis, int per) {
  Loc[id]->add_infectious(dis, per);
  if (Verbose > 2) {
    fprintf(Statusfp, "place %d I %d\n", id, Loc[id]->get_I(dis));
    fflush(Statusfp);
  }
}

void delete_infectious_from_place(int id, int dis, int per) {
  if (Verbose > 2) {
    fprintf(Statusfp, "delete infectious person %d from place %d I %d\n",
	    per, id, Loc[id]->get_I(dis));
    fprintf(Statusfp, "place %d I %d\n", id, Loc[id]->get_I(dis));
    fflush(Statusfp);
  }
  Loc[id]->delete_infectious(dis, per);
  if (Verbose > 2) {
    fprintf(Statusfp, "place %d I %d\n", id, Loc[id]->get_I(dis));
    fflush(Statusfp);
  }
}

char get_type_of_place(int id) {
  return Loc[id]->get_type();
}

int location_should_be_open(int loc, int dis, int day) {
  return Loc[loc]->should_be_open(day, dis);
}

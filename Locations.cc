/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Locations.cc
//

#include "Locations.h"
#include <set>
#include <new>
#include "Population.h"
#include "Strain.h"
#include "Global.h"
#include "Place.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Office.h"
#include "Community.h"
#include "Neighborhood.h"
#include "Household.h"
#include "Hospital.h"
#include "Params.h"
#include "Person.h"

// global singleton object
Locations Loc;

void Locations::get_location_parameters() {
}


void Locations::setup_locations() {
  FILE *fp;
  char location_file[256];
  if (Verbose) {
    fprintf(Statusfp, "setup locations entered\n"); fflush(Statusfp);
  }
  get_param((char *) "locfile", locfile);
  sprintf(location_file, "%s/%s", Population_directory, locfile);
  fp = fopen(location_file, "r");
  if (fp == NULL) {
    fprintf(Statusfp, "location file %s not found\n", location_file);
    abort();
  }
  if (1!=fscanf(fp, "Locations = %d", &locations)){
    fprintf(Statusfp, "failed to parse locations\n");
    abort();
  }
  if (Verbose) {
    fprintf(Statusfp, "Locations = %d\n", locations); fflush(Statusfp);
  }
  locations++; 					// add space for community
  location = new (nothrow) Place * [locations];
  assert(location != NULL);

  int loc = 0;

  // special case: set up the community (which should not be in the locfile)
  community = new (nothrow) Community(0, "Community", 0.0, 0.0, NULL, &Pop);
  location[loc] = community;
  location_map[0] = loc;
  loc++;

  while (loc < locations) {
    int id;
    char s[32];
    char loctype;
    double lon, lat;
    int container_id;
    // fprintf(Statusfp, "reading location %d\n", loc); fflush(Statusfp);
    if (fscanf(fp, "%d %s %c %lf %lf %d",
               &id, s, &loctype, &lat, &lon, &container_id) != 6) {
      fprintf(Statusfp, "Help! Read failure for location %d\n", loc);
      abort();
    }
    if (loctype != COMMUNITY) {
      assert(location_map.find(id) == location_map.end());
      location_map[id] = loc;
    }

    // printf("loctype = %c\n", loctype); fflush(stdout);
    Place *place;
    Place *container = get_location(container_id);
    if (loctype == HOUSEHOLD) {
      place = new (nothrow) Household(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == NEIGHBORHOOD) {
      place = new (nothrow) Neighborhood(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == SCHOOL) {
      place = new (nothrow) School(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == CLASSROOM) {
      place = new (nothrow) Classroom(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == WORKPLACE) {
      place = new (nothrow) Workplace(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == OFFICE) {
      place = new (nothrow) Office(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == HOSPITAL) {
      place = new (nothrow) Hospital(id, s, lon, lat, container, &Pop);
    }
    else if (loctype == COMMUNITY) {
      // Community on loc file is deprecated.  Skip this line;
      locations--;
      continue;
      // community = new (nothrow) Community(id, s, lon, lat, container, &Pop);
      // place = community;
    }
    else {
      printf ("Help! bad loctype = %c\n", loctype);
      abort();
    }
    if (place == NULL) {
      printf("Help! allocation failure for loc %d\n", loc); abort();
    }
    location[loc] = place;
    loc++;
  }
  fclose(fp);
  
  if (Verbose) {
    fprintf(Statusfp, "setup locations finished: Locations = %d\n", locations);
    fflush(Statusfp);
  }
}

void Locations::reset_locations(int run) {
  if (Verbose) {
    fprintf(Statusfp, "reset locations entered\n"); fflush(Statusfp);
  }
  for (int loc = 0; loc < locations; loc++) {
    location[loc]->reset();
  }
  if (Verbose) {
    fprintf(Statusfp, "reset locations finished\n"); fflush(Statusfp);
  }
}

void Locations::update(int day) {
  if (Verbose>1) {
    fprintf(Statusfp, "update locations entered\n"); fflush(Statusfp);
  }
  for (int loc = 0; loc < locations; loc++) {
    location[loc]->update(day);
  }
  if (Verbose>1) {
    fprintf(Statusfp, "update locations finished\n"); fflush(Statusfp);
  }
}

int Locations::get_open_status(int loc, int day) {
  return location[loc]->is_open(day);
}

int Locations::location_should_be_open(int loc, int strain, int day) {
  return location[loc]->should_be_open(day, strain);
}

Place * Locations::get_location(int loc) {
  if (loc == -1) return NULL;
  assert(location_map.find(loc) != location_map.end());
  return location[location_map[loc]];
}

void Locations::location_quality_control() {
  if (Verbose) {
    fprintf(Statusfp, "location quality control check\n"); fflush(Statusfp);
  }
  
  for (int loc = 1; loc < locations; loc++) {
    if (location[loc]->get_size() < 1) {
      fprintf(Statusfp, "Help!  No one visits location %d\n", loc);
      location[loc]->print(0);
      continue;
    }
  }
  
  /*
  if (Verbose) {
    for (int loc = 0; loc < locations; loc++) {
      Place *p = location[loc];
      if (p->get_type() == HOUSEHOLD) {
	if (p->get_HoH() != NULL) { 
	  // printf("household %d HoH %d age %d\n",
	  // p->get_id(),p->get_HoH()->get_id(),p->get_HoH()->get_age());
	  // fflush(stdout);
	}
	else {
	  printf("household %d HoH NULL\n", p->get_id()); fflush(stdout);
	  abort();
	}
      }
    }
  }
  */
  
  if (Verbose) {
    int count[20];
    int total = 0;
    // size distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == HOUSEHOLD) {
        int n = location[loc]->get_size();
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
    // adult distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == HOUSEHOLD) {
        int n = location[loc]->get_adults();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Statusfp, "\nHousehold adult size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }
  
  if (Verbose) {
    int count[20];
    int total = 0;
    // children distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == HOUSEHOLD) {
        int n = location[loc]->get_children();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Statusfp, "\nHousehold children size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }
  
  if (Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households with children
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == HOUSEHOLD) {
        if (location[loc]->get_children() == 0) continue;
        int n = location[loc]->get_adults();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Statusfp, "\nHousehold w/ children, adult size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }
  
  /*
  if (Verbose) {
    int count[20];
    int total = 0;
    // age distribution of heads of households with children
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      Person * per;
      if (location[loc]->get_type() == HOUSEHOLD) {
        if (location[loc]->get_children() == 0) continue;
        if ((per = location[loc]->get_HoH()) == NULL) continue;
        int a = per->get_age();
        int n = a / 10;
        if (n < 20) { count[n]++; }
        else { count[19]++; }
        total++;
      }
    }
    fprintf(Statusfp, "\nAge distribution of heads of households with children: %d households\n", total);
    for (int c = 0; c < 10; c++) {
      fprintf(Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
              10*c, 10*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Statusfp, "\n");
  }
  */

  if (Verbose) {
    int count[20];
    int total = 0;
    // size distribution of schools
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == SCHOOL) {
        int s = location[loc]->get_size();
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
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == CLASSROOM) {
        int s = location[loc]->get_size();
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
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == WORKPLACE) {
        int s = location[loc]->get_size();
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
    for (int loc = 0; loc < locations; loc++) {
      if (location[loc]->get_type() == OFFICE) {
        int s = location[loc]->get_size();
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

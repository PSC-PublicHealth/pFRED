/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Place_List.cc
//

#include "Place_List.h"
#include <set>
#include <new>
#include "Population.h"
#include "Disease.h"
#include "Global.h"
#include "Place.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Office.h"
#include "Neighborhood.h"
#include "Household.h"
#include "Hospital.h"
#include "Params.h"
#include "Person.h"
#include "Grid.h"
#include "Patch.h"
#include "Utils.h"

//// global singleton object
//Place_List Places;

void Place_List::get_parameters() {
}


void Place_List::read_places() {
  FILE *fp;
  char location_file[256];
  int locations;
  double min_lat, max_lat, min_lon, max_lon;
  min_lat = min_lon = 999;
  max_lat = max_lon = -999;

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read places entered\n");
    fflush(Global::Statusfp);
  }
  get_param((char *) "locfile", locfile);
  sprintf(location_file, "%s/%s", Global::Population_directory, locfile);
  fp = fopen(location_file, "r");
  if (fp == NULL) {
    Utils::fred_abort("location file %s not found\n", location_file);
  }
  if (1!=fscanf(fp, "Locations = %d", &locations)){
    Utils::fred_abort("failed to parse location\n");
  }
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "Locations = %d\n", locations);
    fflush(Global::Statusfp);
  }

  char s[80];
  char place_type;
  double lon, lat;
  Place *place = NULL;
  Place *container = NULL;

  // skip header line
  fscanf(fp, "%*s %*s %*s %*s");
  
  int id = (int) places.size();;
  while (fscanf(fp, "%s %c %lf %lf", s, &place_type, &lat, &lon) == 4) {
    if (place_type == 'H' && lat != 0.0) {
      if (lat < min_lat) min_lat = lat;
      if (max_lat < lat) max_lat = lat;
    }
    if (place_type == 'H' && lon != 0.0) {
      if (lon < min_lon) min_lon = lon;
      if (max_lon < lon) max_lon = lon;
    }
    if (place_type == HOUSEHOLD) {
      place = new (nothrow) Household(id, s, lon, lat, container, &Global::Pop);
    }
    else if (place_type == SCHOOL) {
      place = new (nothrow) School(id, s, lon, lat, container, &Global::Pop);
    }
    else if (place_type == WORKPLACE) {
      place = new (nothrow) Workplace(id, s, lon, lat, container, &Global::Pop);
    }
    else if (place_type == HOSPITAL) {
      place = new (nothrow) Hospital(id, s, lon, lat, container, &Global::Pop);
    }
    else {
      Utils::fred_abort("Help! bad place_type %c\n", place_type); 
    }
    if (place == NULL) {
      Utils::fred_abort("Help! allocation failure for place_id %d\n", id); 
    }
    add_place(place);
    place = NULL;
    id++;
  }
  fclose(fp);

  Global::Environment.setup(min_lat, max_lat, min_lon, max_lon);

  int number_places = (int) places.size();
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == HOUSEHOLD) {
      Place *place = places[p];
      double lat = place->get_latitude();
      double lon = place->get_longitude();
      Patch * patch = Global::Environment.get_patch_from_lat_lon(lat,lon);
      if (patch == NULL) {
	double x, y;
	Global::Environment.translate_to_cartesian(lat,lon,&x,&y);
	printf("Help: household %d has bad patch,  lat = %f  lon = %f  x = %f  y = %f\n",
	       place->get_id(),lat,lon,x,y);
      }
      assert(patch != NULL);
      patch->add_household(place);
      place->set_patch(patch);
    }
  }

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read places finished: Places = %d\n", (int) places.size());
    fflush(Global::Statusfp);
  }
}

void Place_List::prepare() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "prepare places entered\n");
    fflush(Global::Statusfp);
  }
  int number_places = places.size();
  for (int p = 0; p < number_places; p++) {
    places[p]->prepare();
  }
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "prepare places finished\n");
    fflush(Global::Statusfp);
  }
}

void Place_List::update(int day) {
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp, "update places entered\n");
    fflush(Global::Statusfp);
  }
  int number_places = places.size();
  for (int p = 0; p < number_places; p++) {
    places[p]->update(day);
  }
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp, "update places finished\n");
    fflush(Global::Statusfp);
  }
}

Place * Place_List::get_place_from_label(char *s) {
  if (strcmp(s, "-1") == 0) return NULL;
  string str;
  str.assign(s);
  if (place_label_map.find(s) != place_label_map.end())
    return places[place_label_map[s]];
  else {
    printf("Help!  can't find place with label = %s\n", s);
    return NULL;
  }
}

void Place_List::add_place(Place * p) {
  // p->print(0);
  assert(place_map.find(p->get_id()) == place_map.end());
  string str;
  str.assign(p->get_label());
  assert(place_label_map.find(str) == place_label_map.end());
  places.push_back(p);
  place_map[p->get_id()] = places.size()-1;
  place_label_map[str] = places.size()-1;
  if (p->get_id() > max_id) max_id = p->get_id();
  // printf("places now = %d\n", (int)(places.size())); fflush(stdout);
}

void Place_List::quality_control() {
  int number_places = places.size();
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "places quality control check for %d places\n", number_places);
    fflush(Global::Statusfp);
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // size distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == HOUSEHOLD) {
        int n = places[p]->get_size();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nHousehold size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        int n = h->get_adults();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nHousehold adult size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // children distribution of households
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        int n = h->get_children();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nHousehold children size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // adult distribution of households with children
    for (int c = 0; c < 15; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        if (h->get_children() == 0) continue;
        int n = h->get_adults();
        if (n < 15) { count[n]++; }
        else { count[14]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nHousehold w/ children, adult size distribution: %d households\n", total);
    for (int c = 0; c < 15; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // age distribution of heads of households with children
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      Person * per;
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        if (h->get_children() == 0) continue;
        if ((per = h->get_HoH()) == NULL) continue;
        int a = per->get_age();
        int n = a / 10;
        if (n < 20) { count[n]++; }
        else { count[19]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households with children: %d households\n", total);
    for (int c = 0; c < 10; c++) {
      fprintf(Global::Statusfp, "age %2d to %d: %6d (%.2f%%)\n",
              10*c, 10*(c+1)-1, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // size distribution of schools
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == SCHOOL) {
        int s = places[p]->get_size();
        int n = s / 50;
        if (n < 20) { count[n]++; }
        else { count[19]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nSchool size distribution: %d schools\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              (c+1)*50, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
  // age distribution in schools
    fprintf(Global::Statusfp, "\nSchool age distribution:\n");
    int count[20];
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == SCHOOL) {
	// places[p]->print(0);
	for (int c = 0; c < 20; c++) {
	  count[c] += ((School *) places[p])->children_in_grade(c);
	}
      }
    }
    for (int c = 0; c < 20; c++) {
      fprintf(Global::Statusfp, "age = %2d  students = %6d\n",
              c, count[c]);;
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[50];
    int total = 0;
    // size distribution of classrooms
    for (int c = 0; c < 50; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == CLASSROOM) {
        int s = places[p]->get_size();
        int n = s;
        if (n < 50) { count[n]++; }
        else { count[50-1]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nClassroom size distribution: %d classrooms\n", total);
    for (int c = 0; c < 50; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[20];
    int total = 0;
    // size distribution of workplaces
    for (int c = 0; c < 20; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == WORKPLACE) {
        int s = places[p]->get_size();
        int n = s / 50;
        if (n < 20) { count[n]++; }
        else { count[19]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nWorkplace size distribution: %d workplaces\n", total);
    for (int c = 0; c < 20; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              (c+1)*50, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  
  if (Global::Verbose) {
    int count[60];
    int total = 0;
    // size distribution of offices
    for (int c = 0; c < 60; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == OFFICE) {
        int s = places[p]->get_size();
        int n = s;
        if (n < 60) { count[n]++; }
        else { count[60-1]++; }
        total++;
      }
    }
    fprintf(Global::Statusfp, "\nOffice size distribution: %d offices\n", total);
    for (int c = 0; c < 60; c++) {
      fprintf(Global::Statusfp, "%3d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "places quality control finished\n");
    fflush(Global::Statusfp);
  }
}

void Place_List::setup_classrooms() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "setup classrooms entered\n");
    fflush(Global::Statusfp);
  }
  int number_places = places.size();
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == SCHOOL) {
      School * school = (School *) places[p];
      school->setup_classrooms();
    }
  }
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "setup classrooms finished\n");
    fflush(Global::Statusfp);
  }
}

void Place_List::setup_offices() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "setup offices entered\n");
    fflush(Global::Statusfp);
  }
  int number_places = places.size();
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == WORKPLACE) {
      Workplace * workplace = (Workplace *) places[p];
      workplace->setup_offices();
    }
  }
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "setup offices finished\n");
    fflush(Global::Statusfp);
  }
}



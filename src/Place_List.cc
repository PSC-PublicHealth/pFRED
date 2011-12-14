/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Place_List.cc
//

#include <math.h>
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
#include "Large_Grid.h"
#include "Small_Grid.h"
#include "Utils.h"
#include "Geo_Utils.h"
#include "Travel.h"
#include "Seasonality.h"
#include "Random.h"

void Place_List::get_parameters() {
}

void Place_List::read_places() {
  double min_lat, max_lat, min_lon, max_lon;
  min_lat = min_lon = 999;
  max_lat = max_lon = -999;

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read places entered\n");
    fflush(Global::Statusfp);
  }
  Params::get_param((char *) "locfile", locfile);
  // read in locations
  char location_file[256];
  sprintf(location_file, "%s/%s", Global::Population_directory, locfile);
  bool use_gzip = false;
  FILE *fp = fopen(location_file, "r");
  if (fp == NULL) {
    // try to find the gzipped version
    char location_file_gz[256];
    sprintf(location_file_gz, "%s/%s.gz", Global::Population_directory, locfile);
    if (fopen(location_file_gz, "r")) {
      char cmd[256];
      use_gzip = true;
      sprintf(cmd, "gunzip -c %s > %s", location_file_gz, location_file);
      system(cmd);
      fp = fopen(location_file, "r");
    }
    // gunzip didn't work ...
    if (fp == NULL) {
      Utils::fred_abort("location_file %s not found\n", location_file);
    }
  }

  char line[256];
  int loc_id = 0;
  while (fgets(line, 255, fp) != NULL) {
    char s[80];
    char place_type;
    double lon, lat;
    Place *place = NULL;
    Place *container = NULL;

    // skip white-space-only lines
    int i = 0;
    while (i < 255 && line[i] != '\0' && isspace(line[i])) i++;
    if (line[i] == '\0') continue;

    // skip comment lines
    if (line[0] == '#') continue;

    if (sscanf(line, "%s %c %lf %lf", s, &place_type, &lat, &lon) == 4) {
      if (place_type == 'H' && lat != 0.0) {
	if (lat < min_lat) min_lat = lat;
	if (max_lat < lat) max_lat = lat;
      }
      if (place_type == 'H' && lon != 0.0) {
	if (lon < min_lon) min_lon = lon;
	if (max_lon < lon) max_lon = lon;
      }
      if (place_type == HOUSEHOLD) {
	place = new (nothrow) Household(loc_id, s, lon, lat, container, &Global::Pop);
      }
      else if (place_type == SCHOOL) {
	place = new (nothrow) School(loc_id, s, lon, lat, container, &Global::Pop);
      }
      else if (place_type == WORKPLACE) {
	place = new (nothrow) Workplace(loc_id, s, lon, lat, container, &Global::Pop);
      }
      else if (place_type == HOSPITAL) {
	place = new (nothrow) Hospital(loc_id, s, lon, lat, container, &Global::Pop);
      }
      else {
	Utils::fred_abort("Help! bad place_type %c\n", place_type); 
      }
      if (place == NULL) {
	Utils::fred_abort("Help! allocation failure for place_id %d\n", loc_id); 
      }
      add_place(place);
      loc_id = places.size();
      place = NULL;
    }
    else {
      Utils::fred_abort("Help! Bad location format for location %d: %s\n",
			loc_id, line);
    }
  }
  fclose(fp);
  if (use_gzip) {
    // remove the uncompressed file
    unlink(location_file);
  }
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "finished reading locations, loc size = %d\n", loc_id);
    fflush(Global::Statusfp);
  }

  // NOTE: Use code below to make projection based on the location file.
  // CURRENT DEFAULT: Use mean US lat-lon (see Geo_Utils.cc)
  // double mean_lat = (min_lat + max_lat) / 2.0;
  // Geo_Utils::set_km_per_degree(mean_lat);

  // create geographical grids
  if (Global::Enable_Large_Grid) {
    Global::Large_Cells = new Large_Grid(min_lon, min_lat, max_lon, max_lat);
    // get coordinates of large grid as alinged to global grid
    min_lon = Global::Large_Cells->get_min_lon();
    min_lat = Global::Large_Cells->get_min_lat();
    max_lon = Global::Large_Cells->get_max_lon();
    max_lat = Global::Large_Cells->get_max_lat();
    // Initialize global seasonality object
    if (Global::Enable_Seasonality) {
      Global::Clim = new Seasonality(Global::Large_Cells);
    }
  }
  Global::Cells = new Grid(min_lon, min_lat, max_lon, max_lat);
  if (Global::Enable_Small_Grid)
    Global::Small_Cells = new Small_Grid(min_lon, min_lat, max_lon, max_lat);

  int number_places = (int) places.size();
  for (int p = 0; p < number_places; p++) {
    if (places[p]->get_type() == HOUSEHOLD) {
      Place *place = places[p];
      double lat = place->get_latitude();
      double lon = place->get_longitude();
      Cell * grid_cell = (Cell *) Global::Cells->get_grid_cell_from_lat_lon(lat,lon);
      if (grid_cell == NULL) {
        printf("Help: household %d has bad grid_cell,  lat = %f  lon = %f\n",
               place->get_id(),lat,lon);
      }
      assert(grid_cell != NULL);
      grid_cell->add_household(place);
      place->set_grid_cell(grid_cell);
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
  if (Global::Enable_Seasonality) {
    Global::Clim->update(day);
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
  // assert(place_map.find(p->get_id()) == place_map.end());
  if (place_map.find(p->get_id()) == place_map.end()) {
    string str;
    str.assign(p->get_label());
    if (place_label_map.find(str) == place_label_map.end()) {
      places.push_back(p);
      place_map[p->get_id()] = places.size()-1;
      place_label_map[str] = places.size()-1;
      if (p->get_id() > max_id) max_id = p->get_id();
      // printf("places now = %d\n", (int)(places.size())); fflush(stdout);
      if (Global::Enable_Local_Workplace_Assignment && p->is_workplace()) {
	workplaces.push_back(p);
      }
    }
    else {
      printf("Warning: duplicate place found: ");
      p->print(0);
    }
  }
  else {
    printf("Warning: duplicate place found: ");
    p->print(0);
  }
}


void Place_List::quality_control(char *directory) {
  int number_places = places.size();
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "places quality control check for %d places\n", number_places);
    fflush(Global::Statusfp);
  }
  
  if (Global::Verbose>1) {
    if (Global::Enable_Large_Grid) {
      char filename [256];
      sprintf(filename, "%s/houses.dat", directory);
      FILE *fp = fopen(filename, "w");
      for (int p = 0; p < number_places; p++) {
	if (places[p]->get_type() == HOUSEHOLD) {
	  double lat = places[p]->get_latitude();
	  double lon = places[p]->get_longitude();
	  double x, y;
	  Global::Large_Cells->translate_to_cartesian(lat, lon, &x, &y);
	  // get coordinates of large grid as alinged to global grid
	  double min_x = Global::Large_Cells->get_min_x();
	  double min_y = Global::Large_Cells->get_min_y();
	  fprintf(fp, "%f %f\n", min_x+x, min_y+y);
	}
      }
      fclose(fp);
    }
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
  
  // relationship between children and decision maker
  if (Global::Verbose > 1) {
    // find adult decision maker for each child
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        if (h->get_children() == 0) continue;
	int size = h->get_size();
	for (int i = 0; i < size; i++) {
	  Person * child = h->get_housemate(i);
	  int ch_age = child->get_age();
	  if (ch_age < 18) {
	    int ch_rel = child->get_relationship();
	    int dm_age = child->get_adult_decision_maker()->get_age();
	    int dm_rel = child->get_adult_decision_maker()->get_relationship();
	    if (dm_rel != 1 || ch_rel != 3) {
	      printf("DECISION_MAKER: household %d %s  decision_maker: %d %d child: %d %d\n",
		     h->get_id(), h->get_label(), dm_age, dm_rel, ch_age, ch_rel);
	    }
	  }
	}
      }
    }
  }
  
  if (Global::Verbose) {
    int count[100];
    int total = 0;
    // age distribution of heads of households
    for (int c = 0; c < 100; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      Person * per = NULL;
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
	for (int i = 0; i < h->get_size(); i++) {
	  if (h->get_housemate(i)->is_householder())
	    per = h->get_housemate(i);
	}
	if (per == NULL) {
	  Utils::fred_warning("Help! No head of household found for household id %d label %s\n",
			    h->get_id(), h->get_label());
	  count[0]++;
	}
	else {
	  int a = per->get_age();
	  if (a < 100) { count[a]++; }
	  else { count[99]++; }
	  total++;
	}
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households: %d households\n", total);
    for (int c = 0; c < 100; c++) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "\n");
  }

  if (Global::Verbose) {
    int count[100];
    int total = 0;
    int children = 0;
    // age distribution of heads of households with children
    for (int c = 0; c < 100; c++) { count[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      Person * per = NULL;
      if (places[p]->get_type() == HOUSEHOLD) {
	Household *h = (Household *) places[p];
        if (h->get_children() == 0) continue;
	children += h->get_children();
	for (int i = 0; i < h->get_size(); i++) {
	  if (h->get_housemate(i)->is_householder())
	    per = h->get_housemate(i);
	}
	if (per == NULL) {
	  Utils::fred_warning("Help! No head of household found for household id %d label %s\n",
			    h->get_id(), h->get_label());
	  count[0]++;
	}
	else {
	  int a = per->get_age();
	  if (a < 100) { count[a]++; }
	  else { count[99]++; }
	  total++;
	}
      }
    }
    fprintf(Global::Statusfp, "\nAge distribution of heads of households with children: %d households\n", total);
    for (int c = 0; c < 100; c++) {
      fprintf(Global::Statusfp, "age %2d: %6d (%.2f%%)\n",
              c, count[c], (100.0*count[c])/total);
    }
    fprintf(Global::Statusfp, "children = %d\n", children);
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
    int covered[4];
    int all[4];
    // distribution of sick leave in workplaces
    for (int c = 0; c < 4; c++) { all[c] = covered[c] = 0; }
    for (int p = 0; p < number_places; p++) {
      if (places[p]->get_type() == WORKPLACE) {
	Workplace * work = (Workplace *) places[p];
        char s = work->get_size_code();
        bool sl = work->is_sick_leave_available();
	switch(s) {
	case 'S':
	  all[0] += s;
	  if (sl) covered[0] += s;
	  break;
	case 'M':
	  all[1] += s;
	  if (sl) covered[1] += s;
	  break;
	case 'L':
	  all[2] += s;
	  if (sl) covered[2] += s;
	  break;
	case 'X':
	  all[3] += s;
	  if (sl) covered[3] += s;
	  break;
	}
      }
    }
    fprintf(Global::Statusfp, "\nWorkplace sick leave coverage: ");
    for (int c = 0; c < 4; c++) {
      fprintf(Global::Statusfp, "%3d: %d/%d %5.2f | ", 
	      c, covered[c], all[c], (all[c]? (1.0*covered[c])/all[c] : 0));
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

Place * Place_List::get_random_workplace() {
  int size = workplaces.size();
  if (size > 0) {
    return workplaces[IRAND(0,size-1)];
  }
  else
    return NULL;
}

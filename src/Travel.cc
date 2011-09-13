/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Travel.cc
//

#include <vector>
#include "Travel.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Large_Grid.h"
#include "Large_Cell.h"
#include "Person.h"
#include "Utils.h"
#include "Geo_Utils.h"
#include <stdio.h>
#include <vector>

typedef vector <Person*> pvec;			// vector of person ptrs
typedef pvec * pvec_ptr;			// pointer to above type
static pvec * travel_list;		   // list of travelers, per day
static pvec_ptr * travel_list_ptr;	      // pointers to above lists
static double mean_trip_duration;		// mean days per trip

#include <vector>
static vector <int> src_row;
static vector <int> src_col;
static vector <int> dest_row;
static vector <int> dest_col;
static vector <int> trip_list;
static int trip_list_size;
static int max_trip_list_size;
static double local_trip_fraction;
static int max_trips_per_day;
static int trips_per_day;
static int min_row;
static int min_col;
static int max_row;
static int max_col;
char tripfile[256];

// runtime parameters
static double * Travel_Duration_Cdf;		// cdf for trip duration
static int max_Travel_Duration = 0;		// number of days in cdf

inline bool inbounds(int r, int c) {
  return (min_row <= r && r <= max_row && min_col <= c && c <=  max_col);
}

void Travel::setup(char * directory) {
  assert(Global::Enable_Large_Grid && Global::Enable_Travel);
  char activefilename[256];
  sprintf(activefilename, "%s/active_trips.txt", directory);
  FILE *outfp = fopen(activefilename,"w");

  min_row = Global::Large_Cells->get_global_row_min();
  max_row = Global::Large_Cells->get_global_row_max();
  min_col = Global::Large_Cells->get_global_col_min();
  max_col = Global::Large_Cells->get_global_col_max();
  int n = 0;
  src_row.clear();
  src_col.clear();
  dest_row.clear();
  dest_col.clear();
  trip_list.clear();

  // get run-time parameters
  Params::get_param((char *) "max_trips_per_day",&max_trips_per_day);
  Params::get_param((char *) "tripfile", tripfile);

  FILE *fp = fopen(tripfile, "r");
  if (fp == NULL) { Utils::fred_abort("Help! Can't open tripfile %s\n", tripfile); }
  int r1,c1,r2,c2;
  int trips_outside = 0;
  max_trip_list_size = 0;
  while (fscanf(fp, "%d %d %d %d", &c1,&r1,&c2,&r2) == 4) {
    max_trip_list_size++;
    if (inbounds(r1,c1) || inbounds(r2,c2)) {
      src_row.push_back(r1);
      src_col.push_back(c1);
      dest_row.push_back(r2);
      dest_col.push_back(c2);
      n++;
      if (!(inbounds(r1,c1)) || !(inbounds(r2,c2))) { trips_outside++;}
    }
  }
  fclose(fp);
  for (int i = 0; i < n; i++) {
    trip_list.push_back(i);
  }
  trip_list_size = n;
  local_trip_fraction = (double) n / (double) max_trip_list_size;
  fprintf(outfp, "boundaries: %d %d %d %d\n", min_col,min_row,max_col,max_row);
  fprintf(outfp, "local_trips %d total_trips %d\n", n, max_trip_list_size);
  fprintf(outfp, "trips outside %d \n", trips_outside);
  for (int i = 0; i < n; i++) {
    fprintf(outfp, "%d %d %d %d\n", src_col[i], src_row[i], dest_col[i], dest_row[i]);
  }
  fclose(outfp);

  // trips_per_day for the modeled region are proportional to the number of trips
  // to-or-from modeled region in the tripfile
  trips_per_day = (int) (max_trips_per_day * local_trip_fraction + 0.5);

  Params::get_param((char *) "travel_duration",&n);
  Travel_Duration_Cdf = new double [n];
  max_Travel_Duration = Params::get_param_vector((char *) "travel_duration",
					 Travel_Duration_Cdf) - 1;
  if (Global::Verbose > 0) {
    for (int i = 0; i <= max_Travel_Duration; i++)
      fprintf(Global::Statusfp,"travel duration = %d %f ",i,Travel_Duration_Cdf[i]);
    fprintf(Global::Statusfp,"\n");
  }
  mean_trip_duration = (max_Travel_Duration + 1) * 0.5;
  
  travel_list = new pvec[max_Travel_Duration+1];
  travel_list_ptr = new pvec_ptr[max_Travel_Duration+1];
  for (int i = 0; i <= max_Travel_Duration; i++) {
    travel_list[i].clear();
    travel_list_ptr[i] = &travel_list[i];
  }
}


void Travel::update_travel(int day) {
  Person * visitor;
  Person * visited;

  if (Global::Enable_Travel == 0)
    return;

  for (int i = 0; i < trips_per_day; i++) {
    select_visitor_and_visited(&visitor, &visited);

    // NOTE: visitor == NULL if the trip is incoming from outside the modeled region
    if (visitor != NULL) {
      // can't start new trip if traveling
      if (visitor->get_travel_status()) continue;
      if (visited != NULL && visited->get_travel_status()) continue;

      // put traveler in travel status
      visitor->start_traveling(visited);

      // put traveler on list for given number of days to travel
      int duration = draw_from_distribution(max_Travel_Duration, Travel_Duration_Cdf);
      // printf("  for %d days\n", duration); fflush(stdout);
      travel_list_ptr[duration]->push_back(visitor);
    }
  }

  // process travelers who are returning home
  // printf("returning home:\n"); fflush(stdout);
  for (unsigned int i = 0; i < travel_list_ptr[0]->size(); i++) {
    visitor = travel_list_ptr[0]->at(i);
    visitor->stop_traveling();
  }
  travel_list_ptr[0]->clear();

  // update days still on travel (after today)
  pvec * tmp = travel_list_ptr[0];
  for (int i = 0; i < max_Travel_Duration; i++) {
    travel_list_ptr[i] = travel_list_ptr[i+1];
  }
  travel_list_ptr[max_Travel_Duration] = tmp;
  return;
}


void Travel::select_visitor_and_visited(Person **v1, Person **v2) {
  static int trip_counter = 0;
  Person * visitor = NULL;
  Person * visited = NULL;
  if (Global::Enable_Travel) {
    // get_trip
    int r1,c1,r2,c2;
    if (trip_counter % trip_list_size == 0) FYShuffle<int>(trip_list);
    int trip = trip_list[trip_counter % trip_list_size];
    trip_counter++;
    r1 = src_row[trip];
    c1 = src_col[trip];
    r2 = dest_row[trip];
    c2 = dest_col[trip];
    assert(inbounds(r1,c1)||inbounds(r2,c2));
    if (Global::Verbose > 2) {
      fprintf(Global::Statusfp, "TRIP %d %d %d %d\n", c1,r1,c2,r2);
    }
    if (inbounds(r1,c1)) {
      Large_Cell * src_cell = Global::Large_Cells->get_grid_cell_with_global_coords(r1,c1);
      visitor = src_cell->select_random_person();
    }
    if (inbounds(r2,c2)) {
      Large_Cell * dest_cell = Global::Large_Cells->get_grid_cell_with_global_coords(r2,c2);
      visited = dest_cell->select_random_person();
    }
    // random pick the direction of the trip
    if (RANDOM() < 0.5) {
      Person * tmp = visitor;
      visitor = visited;
      visited = tmp;
    }
  }
  *v1 = visitor;
  *v2 = visited;
  return;
}


void Travel::quality_control(char * directory) {
}


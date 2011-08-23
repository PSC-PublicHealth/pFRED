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
#include "Geo_Utils.h"
#include <stdio.h>
#include <vector>

static int rows;			 // number of rows in large grid
static int cols;		      // number of columns in large grid
static double *** travel_prob;	   // array of travel prob for each cell
static vector <double> gravity_cdf;		// travel cdf
static vector <int> trip_source;		// source cell index
static vector <int> trip_dest;			// dest cell index
typedef vector <Person*> pvec;			// vector of person ptrs
typedef pvec * pvec_ptr;			// pointer to above type
static pvec * travel_list;		   // list of travelers, per day
static pvec_ptr * travel_list_ptr;	      // pointers to above lists
static double mean_trip_duration;		// mean days per trip

// runtime parameters
static double Travel_Rate = 0.001; // expected fraction of pop on travel
static double * Travel_Duration_Cdf;		// cdf for trip duration
static int Max_Travel_Duration = 0;		// number of days in cdf
static double Min_Travel_Distance = 100.0;	// in km
static double Outside_Travel_Rate = 0.0; // fraction of trips outside model
static int Test_Gravity_Model = 0;	  // if set, exit after sampling

void Travel::setup() {
  assert(Global::Enable_Large_Grid && Global::Enable_Travel);

  // get run-time parameters
  get_param((char *) "travel_rate",&Travel_Rate);
  get_param((char *) "min_travel_distance",&Min_Travel_Distance);
  get_param((char *) "outside_travel_rate",&Outside_Travel_Rate);
  get_param((char *) "test_gravity_model",&Test_Gravity_Model);

  int n;
  get_param((char *) "travel_duration",&n);
  Travel_Duration_Cdf = new double [n];
  Max_Travel_Duration = get_param_vector((char *) "travel_duration",
					 Travel_Duration_Cdf) - 1;
  if (Global::Verbose > 0) {
    for (int i = 0; i <= Max_Travel_Duration; i++)
      fprintf(Global::Statusfp,"travel duration = %d %f ",i,Travel_Duration_Cdf[i]);
    fprintf(Global::Statusfp,"\n");
  }
  mean_trip_duration = (Max_Travel_Duration + 1) * 0.5;
  
  gravity_cdf.clear();
  trip_source.clear();
  trip_dest.clear();
  rows = Global::Large_Cells->get_rows();
  cols = Global::Large_Cells->get_cols();

  travel_list = new pvec[Max_Travel_Duration+1];
  travel_list_ptr = new pvec_ptr[Max_Travel_Duration+1];
  for (int i = 0; i <= Max_Travel_Duration; i++) {
    travel_list[i].clear();
    travel_list_ptr[i] = &travel_list[i];
  }

  // create travel probability matrix
  travel_prob = new double ** [rows];
  for (int i = 0; i < rows; i++) {
    travel_prob[i] = new double * [cols];
    for (int j = 0; j < cols; j++) {
      travel_prob[i][j] = new double [rows*cols];
    }
  }

  // record population size for each large cell
  Global::Large_Cells->set_population_size();

  // setup gravity model probabilities
  double total = 0.0;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      double * trav = travel_prob[row][col];
      Large_Cell * c1 = Global::Large_Cells->get_grid_cell(row, col);
      double x1 = c1->get_center_x();
      double y1 = c1->get_center_y();
      double pop1 = (double) (c1->get_popsize());
      for (int i = 0; i < rows; i++) {
	for (int j = 0; j < cols; j++) {
	  Large_Cell * c2 = Global::Large_Cells->get_grid_cell(i,j);
	  double x2 = c2->get_center_x();
	  double y2 = c2->get_center_y();
	  double pop2 = (double) (c2->get_popsize());
	  double dist = sqrt(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
	  if (dist < Min_Travel_Distance)
	    trav[i*cols+j] = 0.0;
	  else
	    trav[i*cols+j] = (pop1*pop2) / (dist*dist);
	  total += trav[i*cols+j];
	}
      }
    }
  }      

  // make a list of travel probs greater than the threshold
  double threshold = 0.0;
  double active_total = 0.0;
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      double * trav = travel_prob[row][col];
      for (int i = 0; i < rows; i++) {
	for (int j = 0; j < cols; j++) {
	  double pr = trav[i*cols+j] / total;
	  if (pr > threshold) {
	    gravity_cdf.push_back(pr);
	    trip_source.push_back(row*cols+col);
	    trip_dest.push_back(i*cols+j);
	    active_total += pr;
	  }
	}
      }
    }
  }

  // normalize and accumulate probabilities
  double cumulative = 0.0;
  n = gravity_cdf.size();
  for (int i = 0; i < n; i++) {
    gravity_cdf[i] = cumulative + gravity_cdf[i]/active_total;
    cumulative = gravity_cdf[i];
  }

  // free memory for travel probability matrix
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++)
      delete[] travel_prob[i][j];
    delete[] travel_prob[i];
  }
  delete[] travel_prob;

  if (Test_Gravity_Model) test_gravity_model();
}


void Travel::update_travel(int day) {
  Person * visitor;
  Person * visited;

  if (Global::Enable_Travel == 0)
    return;

  // number of new trips
  int trips = Travel_Rate * Global::Pop.get_pop_size() / mean_trip_duration;
  int trips_outside_region = Outside_Travel_Rate * trips;
  int trips_within_region = trips - trips_outside_region;

  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp,
	    "trips within region = %d  trip outside region = %d  total trips = %d\n",
	    trips_within_region, trips_outside_region, trips);
    fflush(Global::Statusfp);
  }

  // trips within region
  // printf("trips within region:\n"); fflush(stdout);
  for (int i = 0; i < trips_within_region; i++) {
    select_visitor_and_visited(&visitor, &visited);
    assert(visitor != NULL);
    assert(visited != NULL);
    // can't start new trip if traveling
    if (visitor->get_travel_status()) continue;
    if (visited->get_travel_status()) continue;

    // put traveler in travel status
    visitor->start_traveling(visited);

    // put traveler on list for given number of days to travel
    int duration = draw_from_distribution(Max_Travel_Duration, Travel_Duration_Cdf);
    // printf("  for %d days\n", duration); fflush(stdout);
    travel_list_ptr[duration]->push_back(visitor);
  }

  // trips outside region
  // printf("trips outside region:\n"); fflush(stdout);
  for (int i = 0; i < trips_outside_region; i++) {
    // select a random person to travel
    visitor = Global::Pop.select_random_person();

    // can't start new trip if traveling
    if (visitor->get_travel_status()) continue;

    // put traveler in travel status
    visitor->start_traveling(NULL);

    // put traveler on list for given number of days to travel
    int duration = draw_from_distribution(Max_Travel_Duration, Travel_Duration_Cdf);
    // printf("  for %d days\n", duration); fflush(stdout);
    travel_list_ptr[duration]->push_back(visitor);
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
  for (int i = 0; i < Max_Travel_Duration; i++) {
    travel_list_ptr[i] = travel_list_ptr[i+1];
  }
  travel_list_ptr[Max_Travel_Duration] = tmp;

  return;
}


void Travel::select_visitor_and_visited(Person **v1, Person **v2) {
  Person * visitor = NULL;
  Person *visited = NULL;
  int n = draw_from_cdf_vector(gravity_cdf);
  int src = trip_source[n];
  int isrc = src/cols;
  int jsrc = src - isrc*cols;
  Large_Cell * src_cell = Global::Large_Cells->get_grid_cell(isrc,jsrc);
  visitor = src_cell->select_random_person();
  assert(visitor != NULL);

  int dest = trip_dest[n];
  int idest = dest/cols;
  int jdest = dest - idest*cols;
  Large_Cell * dest_cell = Global::Large_Cells->get_grid_cell(idest,jdest);
  visited = dest_cell->select_random_person();
  assert(visited != NULL);
  *v1 = visitor;
  *v2 = visited;
  return;
}


void Travel::test_gravity_model() {
  Person * visitor;
  Person * visited;
  double min_lat = Global::Large_Cells->get_min_lat();
  double min_lon = Global::Large_Cells->get_min_lon();
  FILE *fp = fopen("trips.plt", "w");
  FILE *fp2 = fopen("point.txt", "w");
  for (int i = 0; i < 1000; i++) {
    // printf("trial = %d\n", i);fflush(stdout);
    select_visitor_and_visited(&visitor, &visited);
    assert(visitor != NULL);
    assert(visited != NULL);
    printf("visitor id %d\n", visitor->get_id()); fflush(stdout);
    printf("visited id %d\n", visited->get_id()); fflush(stdout);
    double lat1 = visitor->get_household()->get_latitude();
    double lon1 = visitor->get_household()->get_longitude();
    double lat2 = visited->get_household()->get_latitude();
    double lon2 = visited->get_household()->get_longitude();
    double x1, y1;
    double x2, y2;
    Geo_Utils::translate_to_cartesian(lat1, lon1, &x1, &y1, min_lat, min_lon);
    Geo_Utils::translate_to_cartesian(lat2, lon2, &x2, &y2, min_lat, min_lon);
    fprintf(fp, "set arrow from %f,%f to %f,%f nohead\n",x1,y1,x2,y2);
    fprintf(fp2, "%f %f to %f %f\n",x1,y1,x2,y2);
  }
  fprintf(fp, "plot 'point.txt' using 1:2 with dots\n");
  fclose(fp);
  fclose(fp2);
  exit(0);
}


void Travel::quality_control(char * directory) {
  assert(Global::Enable_Large_Grid && Global::Enable_Travel);
  /*
    FILE *fp;
    char filename[1024];
    sprintf(filename, "%s/gravity_probs.txt", directory);
    fp = fopen(filename,"w");
    fclose(fp);
  */
}


/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Place_List.h"
#include "Place.h"
#include "Grid.h"
#include "Params.h"
#include "Random.h"
#include "Utils.h"

// global singleton object
Grid Environment;

void Grid::setup(double minlat, double maxlat, double minlon, double maxlon) {
  get_parameters();
  min_lat  = minlat;
  max_lat  = maxlat;
  min_lon  = minlon;
  max_lon  = maxlon;
  printf("min_lat = %f\n", min_lat);
  printf("max_lat = %f\n", max_lat);
  printf("min_lon = %f\n", min_lon);
  printf("max_lon = %f\n", max_lon);
  fflush(stdout);

  min_x = 0.0;
  max_x = (max_lon-min_lon)*km_per_deg_longitude;
  min_y = 0.0;
  max_y = (max_lat-min_lat)*km_per_deg_latitude;
  rows = 1 + (int) (max_y/patch_size);
  cols = 1 + (int) (max_x/patch_size);
  if (Verbose) {
    printf("rows = %d  cols = %d\n",rows,cols);
    printf("max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(stdout);
  }
  patch = new Patch*[rows];
  for (int i = 0; i < rows; i++) {
    patch[i] = new Patch[cols];
  }
  
  patch_pop = new int*[rows];
  for (int i = 0; i < rows; i++) {
    patch_pop[i] = new int[cols];
  }
  
  patch_prob = new double*[rows];
  for (int i = 0; i < rows; i++) {
    patch_prob[i] = new double[cols];
  }
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      patch[i][j].setup(this,i,j,j*patch_size,(j+1)*patch_size,
			(rows-i-1)*patch_size,(rows-i)*patch_size);
    }
  }

  if (Verbose > 1) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	printf("print patch[%d][%d]:\n",i,j);
	patch[i][j].print();
      }
    }
  }
}

void Grid::get_parameters() {
  get_param((char *) "km_per_deg_longitude", &km_per_deg_longitude);
  get_param((char *) "km_per_deg_latitude", &km_per_deg_latitude);
  get_param((char *) "patch_size", &patch_size);
}

void Grid::make_neighborhoods() {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].make_neighborhood();
    }
  }
}

void Grid::record_favorite_places() {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].record_favorite_places();
      patch_pop[row][col] = patch[row][col].get_neighborhood()->get_size();
    }
  }
}

Patch * Grid::get_patch(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &patch[row][col];
  else
    return NULL;
}


Patch * Grid::select_random_patch() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &patch[row][col];
}


Patch * Grid::get_patch_from_cartesian(double x, double y) {
  int row, col;
  row = rows-1 - (int) (y/patch_size);
  col = (int) (x/patch_size);
  // printf("x = %f y = %f, row = %d col = %d\n",x,y,row,col);
  return get_patch(row, col);
}


Patch * Grid::get_patch_from_lat_lon(double lat, double lon) {
  double x, y;
  translate_to_cartesian(lat,lon,&x,&y);
  return get_patch_from_cartesian(x,y);
}


void Grid::translate_to_cartesian(double lat, double lon, double *x, double *y) {
  *x = (lon - min_lon) * km_per_deg_longitude;
  *y = (lat - min_lat) * km_per_deg_latitude;
}


void Grid::translate_to_lat_lon(double x, double y, double *lat, double *lon) {
  *lon = min_lon + x * km_per_deg_longitude;
  *lat = min_lat + y * km_per_deg_latitude;
}


Patch ** Grid::get_neighbors(int row, int col) {
  Patch ** neighbors = new Patch*[9];
  int n = 0;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      neighbors[n++] = get_patch(i,j);
    }
  }
  return neighbors;
}


Patch * Grid::select_patch_by_gravity_model(int row, int col) {
  // compute patch probabilities for gravity model
  Patch * p1 = &patch[row][col];
  double x1 = p1->get_center_x();
 double y1 = p1->get_center_y();

  double total = 0.0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Patch * p2 = &patch[i][j];
      double x2 = p2->get_center_x();
      double y2 = p2->get_center_y();
      double pop2 = (double) p2->get_neighborhood()->get_size();
      double dist = ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
      if (dist == 0.0) {
	dist = 0.5*patch_size*0.5*patch_size;
      }
      patch_prob[i][j] = pop2 / dist;
      total += patch_prob[i][j];
    }
  }

  // select a patch at random using the computed probabilities
  double r = RANDOM()*total;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (r < patch_prob[i][j]) {
	return &patch[i][j];
      }
      else {
	r -= patch_prob[i][j];
      }
    }
  }
  Utils::fred_abort("Help! patch gravity model failed.\n");
}

void Grid::test_gravity_model() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      for (int n = 0; n < 1000; n++) {
	Patch * p1 = &patch[i][j];
	Patch * p2 = select_patch_by_gravity_model(i,j);
	double x1 = p1->get_center_x();
	double y1 = p1->get_center_y();
	double x2 = p2->get_center_x();
	double y2 = p2->get_center_y();
	double dist = ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	printf("DIST: %f\n", dist);
      }
    }
  }
  exit(0);
}

void Grid::quality_control() {
  if (Verbose) {
    fprintf(Statusfp, "patches quality control check\n");
    fflush(Statusfp);
  }
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].quality_control();
    }
  }

  if (Verbose) {
    fprintf(Statusfp, "patches quality control finished\n");
    fflush(Statusfp);
  }
}



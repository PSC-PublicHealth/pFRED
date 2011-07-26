/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Geo_Utils.h"
#include "Large_grid.h"
#include "Large_cell.h"
#include "Place_List.h"
#include "Place.h"
#include "Params.h"
#include "Random.h"
#include "Utils.h"
#include "Household.h"
#include "Population.h"
#include "Date.h"

Large_grid::Large_grid(double minlon, double minlat, double maxlon, double maxlat) {
  min_lon  = minlon;
  min_lat  = minlat;
  max_lon  = maxlon;
  max_lat  = maxlat;
  printf("min_lon = %f\n", min_lon);
  printf("min_lat = %f\n", min_lat);
  printf("max_lon = %f\n", max_lon);
  printf("max_lat = %f\n", max_lat);
  fflush(stdout);

  get_parameters();
  min_x = 0.0;
  max_x = (max_lon-min_lon)*Geo_Utils::km_per_deg_longitude;
  min_y = 0.0;
  max_y = (max_lat-min_lat)*Geo_Utils::km_per_deg_latitude;
  rows = 1 + (int) (max_y/grid_cell_size);
  cols = 1 + (int) (max_x/grid_cell_size);
  if (Global::Verbose) {
    printf("rows = %d  cols = %d\n",rows,cols);
    printf("max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(stdout);
  }

  grid = new Large_cell * [rows];
  for (int i = 0; i < rows; i++) {
    grid[i] = new Large_cell[cols];
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      grid[i][j].setup(this,i,j,j*grid_cell_size,(j+1)*grid_cell_size,
		       (rows-i-1)*grid_cell_size,(rows-i)*grid_cell_size);
    }
  }

  if (Global::Verbose > 1) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	printf("print grid[%d][%d]:\n",i,j);
	grid[i][j].print();
      }
    }
  }
}

void Large_grid::get_parameters() {
  get_param((char *) "grid_large_cell_size", &grid_cell_size);
}

Large_cell ** Large_grid::get_neighbors(int row, int col) {
  Large_cell ** neighbors = new Large_cell*[9];
  int n = 0;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      neighbors[n++] = get_grid_cell(i,j);
    }
  }
  return neighbors;
}


Large_cell * Large_grid::get_grid_cell(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &grid[row][col];
  else
    return NULL;
}


Large_cell * Large_grid::select_random_grid_cell() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &grid[row][col];
}


Large_cell * Large_grid::get_grid_cell_from_cartesian(double x, double y) {
  int row, col;
  row = rows-1 - (int) (y/grid_cell_size);
  col = (int) (x/grid_cell_size);
  // printf("x = %f y = %f, row = %d col = %d\n",x,y,row,col);
  return get_grid_cell(row, col);
}


Large_cell * Large_grid::get_grid_cell_from_lat_lon(double lat, double lon) {
  double x, y;
  translate_to_cartesian(lat,lon,&x,&y);
  return get_grid_cell_from_cartesian(x,y);
}


void Large_grid::translate_to_cartesian(double lat, double lon, double *x, double *y) {
  *x = (lon - min_lon) * Geo_Utils::km_per_deg_longitude;
  *y = (lat - min_lat) * Geo_Utils::km_per_deg_latitude;
}


void Large_grid::translate_to_lat_lon(double x, double y, double *lat, double *lon) {
  *lon = min_lon + x * Geo_Utils::km_per_deg_longitude;
  *lat = min_lat + y * Geo_Utils::km_per_deg_latitude;
}


void Large_grid::quality_control() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control check\n");
    fflush(Global::Statusfp);
  }
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      grid[row][col].quality_control();
    }
  }
  
  FILE *fp;
  fp = fopen("large_grid.dat", "w");
  for (int row = 0; row < rows; row++) {
    if (row%2) {
      for (int col = cols-1; col >= 0; col--) {
	double x = grid[row][col].get_center_x();
	double y = grid[row][col].get_center_y();
	fprintf(fp, "%f %f\n",x,y);
      }
    }
    else {
      for (int col = 0; col < cols; col++) {
	double x = grid[row][col].get_center_x();
	double y = grid[row][col].get_center_y();
	fprintf(fp, "%f %f\n",x,y);
      }
    }
  }
  fclose(fp);
  
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control finished\n");
    fflush(Global::Statusfp);
  }
}


// Specific to Large_cell Large_grid:

Large_cell * Large_grid::select_grid_cell_by_gravity_model(int row, int col) {
  /*
  // compute grid_cell probabilities for gravity model
  Large_cell * p1 = &grid[row][col];
  double x1 = p1->get_center_x();
  double y1 = p1->get_center_y();
  
  double total = 0.0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Large_cell * p2 = &grid[i][j];
      double x2 = p2->get_center_x();
      double y2 = p2->get_center_y();
      double pop2 = (double) p2->get_neighborhood()->get_size();
      double dist = ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
      if (dist == 0.0) {
	dist = 0.5*grid_cell_size*0.5*grid_cell_size;
      }
      grid_cell_prob[i][j] = pop2 / dist;
      total += grid_cell_prob[i][j];
    }
  }
  
  // select a grid_cell at random using the computed probabilities
  double r = RANDOM()*total;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (r < grid_cell_prob[i][j]) {
	return &grid[i][j];
      }
      else {
	r -= grid_cell_prob[i][j];
      }
    }
  }
  Utils::fred_abort("Help! grid_cell gravity model failed.\n");

  //Will never get here, but will stop compiler warning
  */
  return NULL;
}

void Large_grid::test_gravity_model() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      for (int n = 0; n < 1000; n++) {
	Large_cell * p1 = &grid[i][j];
	Large_cell * p2 = select_grid_cell_by_gravity_model(i,j);
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


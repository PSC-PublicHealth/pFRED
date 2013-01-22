/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Large_Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Geo_Utils.h"
#include "Large_Grid.h"
#include "Large_Cell.h"
#include "Place_List.h"
#include "Place.h"
#include "Params.h"
#include "Random.h"
#include "Utils.h"
#include "Household.h"
#include "Population.h"
#include "Date.h"
#include "DB.h"

Large_Grid::Large_Grid(fred::geo minlon, fred::geo minlat, fred::geo maxlon, fred::geo maxlat) {
  min_lon  = minlon;
  min_lat  = minlat;
  max_lon  = maxlon;
  max_lat  = maxlat;
  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "Large_Grid min_lon = %f\n", min_lon);
    fprintf(Global::Statusfp, "Large_Grid min_lat = %f\n", min_lat);
    fprintf(Global::Statusfp, "Large_Grid max_lon = %f\n", max_lon);
    fprintf(Global::Statusfp, "Large_Grid max_lat = %f\n", max_lat);
    fflush(Global::Statusfp);
  }

  get_parameters();

  // find the global x,y coordinates of SW corner of grid
  min_x = Geo_Utils::get_x(min_lon);
  min_y = Geo_Utils::get_y(min_lat);

  // find the global row and col in which SW corner occurs
  global_row_min = (int) (min_y / grid_cell_size);
  global_col_min = (int) (min_x / grid_cell_size);

  // align coords to global grid
  min_x = global_col_min * grid_cell_size;
  min_y = global_row_min * grid_cell_size;

  // compute lat,lon of SW corner of aligned grid
  min_lat = Geo_Utils::get_latitude(min_y);
  min_lon = Geo_Utils::get_longitude(min_x);

  // find x,y coords of NE corner of bounding box
  max_x = Geo_Utils::get_x(max_lon);
  max_y = Geo_Utils::get_y(max_lat);
  
  // find the global row and col in which NE corner occurs
  global_row_max = (int) (max_y / grid_cell_size);
  global_col_max = (int) (max_x / grid_cell_size);

  // align coords_y to global grid
  max_x = (global_col_max + 1) * grid_cell_size;
  max_y = (global_row_max + 1) * grid_cell_size;

  // compute lat,lon of NE corner of aligned grid
  max_lat = Geo_Utils::get_latitude(max_y);
  max_lon = Geo_Utils::get_longitude(max_x);

  // number of rows and columns needed
  rows = global_row_max - global_row_min + 1;
  cols = global_col_max - global_col_min + 1;

  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "Large_Grid new min_lon = %f\n", min_lon);
    fprintf(Global::Statusfp, "Large_Grid new min_lat = %f\n", min_lat);
    fprintf(Global::Statusfp, "Large_Grid new max_lon = %f\n", max_lon);
    fprintf(Global::Statusfp, "Large_Grid new max_lat = %f\n", max_lat);
    fprintf(Global::Statusfp, "Large_Grid rows = %d  cols = %d\n",rows,cols);
    fprintf(Global::Statusfp, "Large_Grid min_x = %f  min_y = %f\n",min_x,min_y);
    fprintf(Global::Statusfp, "Large_Grid max_x = %f  max_y = %f\n",max_x,max_y);
    fprintf(Global::Statusfp, "Large_Grid global_col_min = %d  global_row_min = %d\n",
        global_col_min, global_row_min);
    fflush(Global::Statusfp);
  }

  grid = new Large_Cell * [rows];
  for (int i = 0; i < rows; i++) {
    grid[i] = new Large_Cell[cols];
  }
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      grid[i][j].setup(this,i,j);
      if (Global::Verbose > 1) {
        printf("print grid[%d][%d]:\n",i,j); fflush(stdout);
        grid[i][j].print();
      }
      //printf( "row = %d col = %d id = %d\n", i, j, grid[i][j].get_id() );
    }
  }
}

void Large_Grid::get_parameters() {
  Params::get_param_from_string("grid_large_cell_size", &grid_cell_size);
}

Large_Cell * Large_Grid::get_grid_cell(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &grid[row][col];
  else
    return NULL;
}


Large_Cell * Large_Grid::get_grid_cell(fred::geo lat, fred::geo lon) {
  int row = get_row(lat);
  int col = get_col(lon);
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &grid[row][col];
  else
    return NULL;
}


Large_Cell * Large_Grid::get_grid_cell_with_global_coords(int row, int col) {
  return get_grid_cell(row-global_row_min, col-global_col_min);
}

Large_Cell * Large_Grid::get_grid_cell_from_id( int id ) {
  int row = id / cols;
  int col = id % cols;
  FRED_VERBOSE( 4, "grid cell lookup for id = %d ... calculated row = %d, col = %d, rows = %d, cols = %d\n", id, row, col, rows, cols );
  assert( grid[ row ][ col ].get_id() == id );
  return &( grid[ row ][ col ] );
}

Large_Cell * Large_Grid::select_random_grid_cell() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &grid[row][col];
}


void Large_Grid::quality_control(char * directory) {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control check\n");
    fflush(Global::Statusfp);
  }

  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      grid[row][col].quality_control();
    }
  }

  if (Global::Verbose>1) {
    char filename [FRED_STRING_SIZE];
    sprintf(filename, "%s/large_grid.dat", directory);
    FILE *fp = fopen(filename, "w");
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
  }

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control finished\n");
    fflush(Global::Statusfp);
  }
}


// Specific to Large_Cell Large_Grid:

void Large_Grid::set_population_size() {
  int pop_size = Global::Pop.get_pop_size();
  for (int p = 0; p < pop_size; p++) {
    Person *per = Global::Pop.get_person_by_index(p);
    Place * h = per->get_household();
    assert (h != NULL);
    int row = get_row(h->get_latitude());
    int col = get_col(h->get_longitude());
    Large_Cell * cell = get_grid_cell(row,col);
    cell->add_person(per);
  }

  // print debugging data for large grid
  /*
  FILE *fp;
  char filename[FRED_STRING_SIZE];
  sprintf(filename, "OUT/largegrid.txt");
  fp = fopen(filename,"w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Large_Cell * cell = get_grid_cell(i,j);
      double x = cell->get_center_x();
      double y = cell->get_center_y();
      fred::geo lat = Geo_Utils::get_latitude(y);
      fred::geo lon = Geo_Utils::get_longitude(x);
      int popsize = cell->get_popsize();
      fprintf(fp, "%d %d %f %f %f %f %d\n", i,j,x,y,lon,lat,popsize);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
  */
}

void Large_Grid::read_max_popsize() {
  int r,c, n;
  char filename[FRED_STRING_SIZE];
  if (Global::Enable_Travel) {
    Params::get_param_from_string("cell_popfile", filename);
    FILE *fp = Utils::fred_open_file(filename);
    if (fp == NULL) {
      Utils::fred_abort("Help! Can't open cell_pop_file %s\n", filename);
    }
    printf("reading %s\n", filename);
    while (fscanf(fp, "%d %d %d ", &c,&r,&n) == 3) {
      Large_Cell * cell = get_grid_cell_with_global_coords(r,c);
      if (cell != NULL) {
        cell->set_max_popsize(n);
      }
    }
    fclose(fp);
    printf("finished reading %s\n", filename);
  }
}
//<<<<<<< Large_Grid.cc

void Large_Grid::report_grid_stats( int day ) {
  for ( int dis = 0; dis < Global::Diseases; ++dis ) {
    #pragma omp parallel for schedule(runtime)
    for ( int r = 0; r < get_rows(); ++r ) {
      for ( int c = 0; c < get_cols(); ++c ) {
        Global::db.enqueue_transaction(
            grid[ r ][ c ].collect_cell_stats( day, dis ) );
      }
    }
  }
}

//=======

Place *Large_Grid::get_nearby_workplace(int row, int col, double x, double y, int min_staff, int max_staff, double * min_dist) {
  // find nearest workplace that has right number of employees
  Place * nearby_workplace = NULL;
  *min_dist = 1e99;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      Large_Cell * cell = get_grid_cell(i,j);
      if (cell != NULL) {
	Place * closest_workplace = cell->get_closest_workplace(x,y,min_staff,max_staff,min_dist);
	if (closest_workplace != NULL) {
	  nearby_workplace = closest_workplace;
	}
      }
    }
  }
  return nearby_workplace;
}


//>>>>>>> 1.13

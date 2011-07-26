/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_grid.h
//

#ifndef _FRED_LARGE_GRID_H
#define _FRED_LARGE_GRID_H

#include <string.h>
#include "Place.h"
class Large_cell;

class Large_grid {
public:
  Large_grid(double minlon, double minlat, double maxlon, double maxlat);
  ~Large_grid() {}
  void get_parameters();
  Large_cell ** get_neighbors(int row, int col);
  Large_cell * get_grid_cell(int row, int col);
  Large_cell * select_random_grid_cell();
  Large_cell * get_grid_cell_from_cartesian(double x, double y);
  Large_cell * get_grid_cell_from_lat_lon(double lat, double lon);
  void translate_to_cartesian(double lat, double lon, double *x, double *y);
  void translate_to_lat_lon(double x, double y, double *lat, double *lon);
  void quality_control();

  // Specific to Large_cell grid:
  Large_cell * select_grid_cell_by_gravity_model(int row, int col);
  void test_gravity_model();

protected:
  Large_cell ** grid;			      // Rectangular array of grid_cells
  int rows;					// number of rows
  int cols;					// number of columns
  double grid_cell_size;			// km per side
  double min_lat;
  double min_lon;
  double max_lat;
  double max_lon;
  double min_x;
  double max_x;
  double min_y;
  double max_y;

  // Specific to Large_cell grid:
};

#endif // _FRED_LARGE_GRID_H

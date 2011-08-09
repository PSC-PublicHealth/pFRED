/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_Grid.h
//

#ifndef _FRED_LARGE_GRID_H
#define _FRED_LARGE_GRID_H

#include <string.h>
#include "Place.h"
class Large_Cell;

class Large_Grid {
public:
  Large_Grid(double minlon, double minlat, double maxlon, double maxlat);
  ~Large_Grid() {}
  void get_parameters();
  Large_Cell ** get_neighbors(int row, int col);
  Large_Cell * get_grid_cell(int row, int col);
  Large_Cell * select_random_grid_cell();
  Large_Cell * get_grid_cell_from_cartesian(double x, double y);
  Large_Cell * get_grid_cell_from_lat_lon(double lat, double lon);
  void set_population_size();
  void quality_control();
  int get_rows() { return rows; }
  int get_cols() { return cols; }
  double get_min_lat() { return min_lat; }
  double get_min_lon() { return min_lon; }

protected:
  Large_Cell ** grid;			      // Rectangular array of grid_cells
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
};

#endif // _FRED_LARGE_GRID_H

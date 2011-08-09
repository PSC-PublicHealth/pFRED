/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Small_Grid.h
//

#ifndef _FRED_SMALL_GRID_H
#define _FRED_SMALL_GRID_H

#include <string.h>
#include "Place.h"
class Small_Cell;

class Small_Grid {
public:
  Small_Grid(double minlon, double minlat, double maxlon, double maxlat);
  ~Small_Grid() {}
  void get_parameters();
  Small_Cell ** get_neighbors(int row, int col);
  Small_Cell * get_grid_cell(int row, int col);
  Small_Cell * select_random_grid_cell();
  Small_Cell * get_grid_cell_from_cartesian(double x, double y);
  Small_Cell * get_grid_cell_from_lat_lon(double lat, double lon);
  void quality_control();

  // Specific to Small_Cell grid:

protected:
  Small_Cell ** grid;			      // Rectangular array of grid_cells
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

  // Specific to Small_Cell grid:
};

#endif // _FRED_SMALL_GRID_H

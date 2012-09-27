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
#include "Abstract_Grid.h"
#include "Global.h"

class Large_Cell;

class Large_Grid : public Abstract_Grid {
public:
  Large_Grid(fred::geo minlon, fred::geo minlat, fred::geo maxlon, fred::geo maxlat);
  ~Large_Grid() {}
  Large_Cell ** get_neighbors(int row, int col);
  Large_Cell * get_grid_cell(int row, int col);
  Large_Cell * get_grid_cell(fred::geo lat, fred::geo lon);
  Large_Cell * get_grid_cell_with_global_coords(int row, int col);
  Large_Cell * select_random_grid_cell();
  Place * get_nearby_workplace(int row, int col, double x, double y, int min_staff, int max_staff, double * min_dist);
  void get_parameters();
  void set_population_size();
  void quality_control(char * directory);
  void read_max_popsize();

protected:
  Large_Cell ** grid;            // Rectangular array of grid_cells
};

#endif // _FRED_LARGE_GRID_H

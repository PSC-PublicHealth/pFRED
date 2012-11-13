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
// File: Large_Grid.h
//

#ifndef _FRED_LARGE_GRID_H
#define _FRED_LARGE_GRID_H

#include <string.h>
#include <math.h>

#include "Place.h"
#include "Abstract_Grid.h"
#include "Global.h"
#include "DB.h"

class Large_Cell;

class Large_Grid : public Abstract_Grid {
public:
  Large_Grid(fred::geo minlon, fred::geo minlat, fred::geo maxlon, fred::geo maxlat);
  ~Large_Grid() {}
  Large_Cell ** get_neighbors(int row, int col);
  Large_Cell * get_grid_cell(int row, int col);
  Large_Cell * get_grid_cell(fred::geo lat, fred::geo lon);
  Large_Cell * get_grid_cell_with_global_coords(int row, int col);
  Large_Cell * get_grid_cell_from_id( int id );
  Large_Cell * select_random_grid_cell();
  Place * get_nearby_workplace(int row, int col, double x, double y, int min_staff, int max_staff, double * min_dist);
  void get_parameters();
  void set_population_size();
  void quality_control(char * directory);
  void read_max_popsize();

  void report_grid_stats( int day );
 

protected:
  Large_Cell ** grid;            // Rectangular array of grid_cells
};

#endif // _FRED_LARGE_GRID_H

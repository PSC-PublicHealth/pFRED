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

#include "Global.h"
#include "Abstract_Grid.h"
class Large_Grid;
class Small_Cell;

class Small_Grid : public Abstract_Grid {
public:
  Small_Grid(Large_Grid * lgrid);
  ~Small_Grid() {}
  void get_parameters();
  Small_Cell ** get_neighbors(int row, int col);
  Small_Cell * get_grid_cell(int row, int col);
  Small_Cell * get_grid_cell(fred::geo lat, fred::geo lon);
  Small_Cell * select_random_grid_cell();
  void quality_control(char * directory);

  // Specific to Small_Cell grid:

protected:
  Small_Cell ** grid;            // Rectangular array of grid_cells
  Large_Grid * large_grid;

  // Specific to Small_Cell grid:
};

#endif // _FRED_SMALL_GRID_H

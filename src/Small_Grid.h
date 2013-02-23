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

  void update(int day);

  void initialize_gaia_data(char * directory, int run);
  void print_gaia_data(char * directory, int run, int day);

protected:
  Small_Cell ** grid;            // Rectangular array of grid_cells
  Large_Grid * large_grid;

  // Specific to Small_Cell grid:
};

#endif // _FRED_SMALL_GRID_H

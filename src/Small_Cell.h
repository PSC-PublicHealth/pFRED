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
// File: Small_Cell.h
//

#ifndef _FRED_SMALL_CELL_H
#define _FRED_SMALL_CELL_H

#include "Abstract_Cell.h"

class Small_Grid;

class Small_Cell : public Abstract_Cell {
public:
  Small_Cell() {}
  ~Small_Cell() {}
  void setup(int i, int j, double grid_cell_size, double grid_min_x, double grid_min_y);
  void quality_control();
  void update(int day) { count = 0; popsize = 0; }
  double distance_to_grid_cell(Small_Cell *grid_cell2);
  int get_count() { return count; }
  int get_popsize() { return popsize; }
  void update_cell_counts(int inf, int total) { count += inf; popsize += total; } 
  void print();

protected:
  int count;
  int popsize;
};

#endif // _FRED_SMALL_CELL_H

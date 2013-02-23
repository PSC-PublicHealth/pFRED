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
// File: Small_Cell.cc
//

#include "Small_Cell.h"
#include "Small_Grid.h"
#include "Utils.h"

void Small_Cell::setup(int i, int j, double grid_cell_size, double grid_min_x, double grid_min_y) {
  row = i;
  col = j;
  min_x = grid_min_x + (col)*grid_cell_size;
  min_y = grid_min_y + (row)*grid_cell_size;
  max_x = grid_min_x + (col+1)*grid_cell_size;
  max_y = grid_min_y + (row+1)*grid_cell_size;
  center_y = (min_y+max_y)/2.0;
  center_x = (min_x+max_x)/2.0;
}

void Small_Cell::quality_control() {
  return;
}

double Small_Cell::distance_to_grid_cell(Small_Cell *p2) {
  double x1 = center_x;
  double y1 = center_y;
  double x2 = p2->get_center_x();
  double y2 = p2->get_center_y();
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

void Small_Cell::print() {
  FRED_VERBOSE(0, "small_cell: %d %d %d %d\n", row, col, count, popsize);
}



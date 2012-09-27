/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Small_Cell.cc
//

#include "Small_Cell.h"
#include "Small_Grid.h"

void Small_Cell::setup(Small_Grid * grd, int i, int j) {
  grid = grd;
  row = i;
  col = j;
  double grid_cell_size = grid->get_grid_cell_size();
  double grid_min_x = grid->get_min_x();
  double grid_min_y = grid->get_min_y();
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






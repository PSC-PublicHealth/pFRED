/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_cell.h
//

#ifndef _FRED_LARGE_CELL_H
#define _FRED_LARGE_CELL_H

class Large_grid;

class Large_cell {
public:
  Large_cell() {}
  ~Large_cell() {}
  void setup(Large_grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax);
  void print();
  void print_coord();
  void quality_control();
  double distance_to_grid_cell(Large_cell *grid_cell2);
  int get_row() { return row; }
  int get_col() { return col; }
  double get_min_x() { return min_x;}
  double get_max_x() { return max_x;}
  double get_min_y() { return min_y;}
  double get_max_y() { return max_y;}
  double get_center_y() { return center_y;}
  double get_center_x() { return center_x;}

protected:
  Large_grid * grid;
  Large_cell ** neighbors;
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
};

#endif // _FRED_LARGE_CELL_H

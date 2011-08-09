/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Small_Cell.h
//

#ifndef _FRED_SMALL_CELL_H
#define _FRED_SMALL_CELL_H

class Small_Grid;

class Small_Cell {
public:
  Small_Cell() {}
  ~Small_Cell() {}
  void setup(Small_Grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax);
  void print();
  void print_coord();
  void quality_control();
  double distance_to_grid_cell(Small_Cell *grid_cell2);
  int get_row() { return row; }
  int get_col() { return col; }
  double get_min_x() { return min_x;}
  double get_max_x() { return max_x;}
  double get_min_y() { return min_y;}
  double get_max_y() { return max_y;}
  double get_center_y() { return center_y;}
  double get_center_x() { return center_x;}

protected:
  Small_Grid * grid;
  Small_Cell ** neighbors;
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
};

#endif // _FRED_SMALL_CELL_H

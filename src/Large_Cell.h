/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_Cell.h
//

#ifndef _FRED_LARGE_CELL_H
#define _FRED_LARGE_CELL_H

#include <vector>
#include "Person.h"

class Large_Grid;

class Large_Cell {
public:
  Large_Cell() {}
  ~Large_Cell() {}
  void setup(Large_Grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax);
  void print();
  void print_coord();
  void quality_control();
  double distance_to_grid_cell(Large_Cell *grid_cell2);
  int get_row() { return row; }
  int get_col() { return col; }
  double get_min_x() { return min_x;}
  double get_max_x() { return max_x;}
  double get_min_y() { return min_y;}
  double get_max_y() { return max_y;}
  double get_center_y() { return center_y;}
  double get_center_x() { return center_x;}
  void add_person(Person *p) { person.push_back(p); popsize++; }
  int get_popsize() { return popsize; }
  Person * select_random_person();
  void set_max_popsize(int n);
  int get_max_popsize() { return max_popsize; }
  double get_pop_density() { return pop_density; }

protected:
  Large_Grid * grid;
  Large_Cell ** neighbors;
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
  int popsize;
  vector <Person *> person;
  int max_popsize;
  double pop_density;
};

#endif // _FRED_LARGE_CELL_H

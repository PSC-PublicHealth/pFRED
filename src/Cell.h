/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cell.h
//

#ifndef _FRED_CELL_H
#define _FRED_CELL_H

#include <vector>
#include <string.h>

#include "Place.h"
class Person;
class Grid;

class Cell {
public:
  Cell() {}
  ~Cell() {}
  void setup(Grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax);
  void print();
  void print_coord();
  void quality_control();
  double get_min_y() { return min_y;}
  double get_min_x() { return min_x;}
  double get_max_y() { return max_y;}
  double get_max_x() { return max_x;}
  double get_center_y() { return center_y;}
  double get_center_x() { return center_x;}
  double distance_to_grid_cell(Cell *grid_cell2);
  int get_row() { return row; }
  int get_col() { return col; }

  // specific to Cell grid:
  void make_neighborhood();
  void add_household(Place *p);
  void record_favorite_places();
  Place *select_neighborhood();
  Person *select_random_person();
  Place *select_random_household();
  Place *select_random_workplace();
  Place *select_random_school(int age);
  Person *select_random_person_from_neighbors();
  int get_houses() { return houses;}
  /// @brief Return list of households in this grid cell.
  vector <Place *> get_households() { return household; }
  Place * get_neighborhood() { return neighborhood; }
  void enroll(Person *per) { neighborhood->enroll(per); }
  int get_occupied_houses() { return occupied_houses; }
  void add_occupied_house() { occupied_houses++; }
  void subtract_occupied_house() { occupied_houses--; }
  int get_target_households() { return target_households; }
  int get_target_popsize() { return target_popsize; }

protected:
  Place * select_neighboring_grid_cell();
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
  Cell ** neighbors;
  Grid * grid;

  int houses;
  Place * neighborhood;
  vector <Place *> household;
  vector <Place *> school[20];
  vector <Place *> workplace;
  vector <Person *> person;

  // target grid_cell variables;
  int target_households;
  int target_popsize;
  int occupied_houses;
};

#endif // _FRED_CELL_H

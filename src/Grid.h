/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Grid.h
//

#ifndef _FRED_GRID_H
#define _FRED_GRID_H

#include <string.h>
#include "Place.h"
class Cell;

class Grid {
public:
  Grid(double minlon, double minlat, double maxlon, double maxlat);
  ~Grid() {}
  void get_parameters();
  Cell ** get_neighbors(int row, int col);
  Cell * get_grid_cell(int row, int col);
  Cell * select_random_grid_cell();
  Cell * get_grid_cell_from_cartesian(double x, double y);
  Cell * get_grid_cell_from_lat_lon(double lat, double lon);
  void translate_to_cartesian(double lat, double lon, double *x, double *y);
  void translate_to_lat_lon(double x, double y, double *lat, double *lon);
  void quality_control();

  // Specific to Cell grid:
  void record_favorite_places();
  void add_vacant_house(Place *house);
  Place * get_vacant_house();
  int get_target_popsize() { return target_popsize; }
  int get_target_households() { return target_households; }
  int get_vacant_houses() { return (int) vacant_houses.size(); }
  void population_migration();
  void select_emigrants();
  void select_immigrants();
  void print_household_distribution(char * dir, char * date_string, int run);

protected:
  Cell ** grid;			      // Rectangular array of grid_cells
  int rows;					// number of rows
  int cols;					// number of columns
  double grid_cell_size;			// km per side
  double min_lat;
  double min_lon;
  double max_lat;
  double max_lon;
  double min_x;
  double max_x;
  double min_y;
  double max_y;

  // Specific to Cell grid:
  int target_popsize;
  int target_households;
  int target_pop_age[100];
  vector <Place *> vacant_houses;
};

#endif // _FRED_GRID_H

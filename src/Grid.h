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
#include "Patch.h"

class Grid {
public:
  Grid() {}
  void setup(double minlat, double maxlat, double minlon, double maxlon);
  void get_parameters();
  void record_favorite_places();
  Patch * get_patch(int row, int col);
  Patch * select_random_patch();
  Patch * get_patch_from_cartesian(double x, double y);
  Patch * get_patch_from_lat_lon(double lat, double lon);
  void translate_to_cartesian(double lat, double lon, double *x, double *y);
  void translate_to_lat_lon(double x, double y, double *lat, double *lon);
  Patch ** get_neighbors(int row, int col);
  Patch * select_patch_by_gravity_model(int row, int col);
  void test_gravity_model();
  void make_neighborhoods();
  void quality_control();

protected:
  Patch ** patch;		 		// array of patches
  int ** patch_pop;			   // array of patch populations
  double ** patch_prob;   // array of patch probablities, for gravity model
  double patch_size; 				// km per side
  double min_lat;
  double min_lon;
  double max_lat;
  double max_lon;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double km_per_deg_longitude;
  double km_per_deg_latitude;
  int rows;
  int cols;
};

#endif // _FRED_GRID_H

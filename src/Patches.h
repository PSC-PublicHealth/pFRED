/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Patches.h
//

#ifndef _FRED_PATCHES_H
#define _FRED_PATCHES_H

#include <string.h>
#include "Patch.h"

class Patches {
public:
  Patches() {}
  void get_parameters();
  void setup(double xmin, double xmax, double ymin, double ymax);
  void reset(int run);
  Patch * get_patch(int row, int col);
  Patch * get_patch_from_cartesian(double x, double y);
  Patch * get_patch_from_lat_lon(double lat, double lon);
  void translate_to_cartesian(double lat, double lon, double *x, double *y);
  void translate_to_lat_lon(double x, double y, double *lat, double *lon);
  Patch ** get_neighbors(int row, int col);
protected:
  Patch ** patch;		 		// array of patches
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

#endif // _FRED_PATCHES_H

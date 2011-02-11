/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Patches.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Locations.h"
#include "Place.h"
#include "Patches.h"
#include "Params.h"

Patches * patches;

void Patches::get_parameters() {
  get_param((char *) "km_per_deg_longitude", &km_per_deg_longitude);
  get_param((char *) "km_per_deg_latitude", &km_per_deg_latitude);
  get_param((char *) "patch_size", &patch_size);
}


void Patches::setup(double xmin, double xmax, double ymin, double ymax) {
  get_parameters();
  min_lon = xmin;
  max_lon = xmax;
  min_lat = ymin;
  max_lat = ymax;
  min_x = 0.0;
  max_x = (max_lon-min_lon)*km_per_deg_longitude;
  min_y = 0.0;
  max_y = (max_lat-min_lat)*km_per_deg_latitude;
  rows = 1 + (int) (max_y/patch_size);
  cols = 1 + (int) (max_x/patch_size);
  if (Verbose > 1) {
    printf("rows = %d  cols = %d\n",rows,cols);
    printf("max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(stdout);
  }
  patch = new Patch*[rows];
  for (int i = 0; i < rows; i++) {
    patch[i] = new Patch[cols];
  }
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      patch[i][j].setup(this,i,j,j*patch_size,(j+1)*patch_size,
			(rows-i-1)*patch_size,(rows-i)*patch_size);
    }
  }

  if (Verbose > 1) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	printf("print patch[%d][%d]:\n",i,j);
	patch[i][j].print();
      }
    }
  }

  // register each household in its patch
  int locations = Loc.get_locations();
  int h = 0;
  for (int i = 0; i < locations; i++) {
    Place * p = Loc.get_location_at_position(i);
    if (p->get_type() == HOUSEHOLD) {
      double lat = p->get_latitude();
      double lon = p->get_longitude();
      Patch * pat = get_patch_from_lat_lon(lat,lon);
      if (pat != NULL) {
	pat->add_household(p);
	h++;
	// printf("ok h = %d\n", h); fflush(stdout);
	p->set_patch(pat);
      }
      else {
	double x, y;
	translate_to_cartesian(lat,lon,&x,&y);
	printf("Help: household %d has bad patch,  lat = %f  lon = %f  x = %f  y = %f\n", p->get_id(),lat,lon,x,y);
      }
    }
  }
  if (Verbose > 1) {
    printf("Registered %d households\n",h);
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      patch[i][j].set_social_networks();
    }
  }

}

void Patches::reset(int run) {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].reset(run);
    }
  }
}

Patch * Patches::get_patch(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &patch[row][col];
  else
    return NULL;
}


Patch * Patches::get_patch_from_cartesian(double x, double y) {
  int row, col;
  row = rows-1 - (int) (y/patch_size);
  col = (int) (x/patch_size);
  // printf("x = %f y = %f, row = %d col = %d\n",x,y,row,col);
  return get_patch(row, col);
}


Patch * Patches::get_patch_from_lat_lon(double lat, double lon) {
  double x, y;
  translate_to_cartesian(lat,lon,&x,&y);
  return get_patch_from_cartesian(x,y);
}


void Patches::translate_to_cartesian(double lat, double lon, double *x, double *y) {
  *x = (lon - min_lon) * km_per_deg_longitude;
  *y = (lat - min_lat) * km_per_deg_latitude;
}


void Patches::translate_to_lat_lon(double x, double y, double *lat, double *lon) {
  *lon = min_lon + x * km_per_deg_longitude;
  *lat = min_lat + y * km_per_deg_latitude;
}


Patch ** Patches::get_neighbors(int row, int col) {
  Patch ** neighbors = new Patch*[9];
  int n = 0;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      neighbors[n++] = get_patch(i,j);
    }
  }
  return neighbors;
}



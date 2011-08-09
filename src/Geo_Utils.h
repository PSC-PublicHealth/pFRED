/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Geo_Utils.h
//

#ifndef FRED_GEO_UTILS_H_
#define FRED_GEO_UTILS_H_

#include <stdio.h>
#include <math.h>

class Geo_Utils{
 public:
  static void set_km_per_degree(double lat);
  static double haversine_distance (double lon1, double lat1, double lon2, double lat2);
  static double spherical_cosine_distance (double lon1, double lat1, double lon2, double lat2);
  static double spherical_projection_distance (double lon1, double lat1, double lon2, double lat2);
  static void translate_to_cartesian(double lat, double lon, double *x, double *y,
				     double min_lat, double min_lon);
  static void translate_to_lat_lon(double x, double y, double *lat, double *lon,
				   double min_lat, double min_lon);
  static double km_per_deg_longitude;
  static double km_per_deg_latitude;
 private:
  // see http://andrew.hedges.name/experiments/haversine/
  static const double DEG_TO_RAD = 0.017453292519943295769236907684886; // PI/180
  static const double EARTH_RADIUS = 6373.0; // earth's radius in kilometers
  static const double KM_PER_DEG_LAT = 111.325; // assuming spherical earth
  
  // Allegheny VALUES - for regression test
  static const double ALLEG_KM_PER_DEG_LON = 84.83063; // assuming spherical earth
  static const double ALLEG_KM_PER_DEG_LAT = 111.04326; // assuming spherical earth
  
  // US Mean latitude-longitude (http://www.travelmath.com/country/United+States)
  static const double MEAN_US_LON = -97.0; // near Wichita, KS
  static const double MEAN_US_LAT = 38.0; // near Wichita, KS
  
  // from http://www.ariesmar.com/degree-latitude.php
  static const double MEAN_US_KM_PER_DEG_LON = 87.832; // at 38 deg N
  static const double MEAN_US_KM_PER_DEG_LAT = 110.996; // 
};

#endif /* FRED_GEO_UTILS_H_ */


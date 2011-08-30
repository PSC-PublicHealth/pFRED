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

  /**
   * Sets the kilometers per degree longitude at a given latitiude
   *
   * @param lat the latitude to set KM / degree
   */
  static void set_km_per_degree(double lat);

  /**
   * @param lon1
   * @param lat1
   * @param lon2
   * @param lat2
   *
   * @return the haversine distance between the two points on the Earth's surface
   */
  static double haversine_distance (double lon1, double lat1, double lon2, double lat2);

  /**
   * @param lon1
   * @param lat1
   * @param lon2
   * @param lat2
   *
   * @return the spherical cosine distance between the two points on the Earth's surface
   */
  static double spherical_cosine_distance (double lon1, double lat1, double lon2, double lat2);

  /**
   * @param lon1
   * @param lat1
   * @param lon2
   * @param lat2
   *
   * @return the spherical projection distance between the two points on the Earth's surface
   */
  static double spherical_projection_distance (double lon1, double lat1, double lon2, double lat2);

  /**
   * Translate a given latitude and longitude to an (x,y) coordinate.
   *
   * @param lat the latitude of the point
   * @param lon the longitude of the point
   * @param x pointer to the x coordinate of the point
   * @param y pointer to the y coordinate of the point
   */
  static void translate_to_cartesian(double lat, double lon, double *x, double *y,
				     double min_lat, double min_lon);

  /**
   * Translate a given (x,y) coordinate to a latitude and longitude.
   *
   * @param x the x coordinate of the point
   * @param y the y coordinate of the point
   * @param lat pointer to the latitude of the point
   * @param lon pointer to the longitude of the point
   */
  static void translate_to_lat_lon(double x, double y, double *lat, double *lon,
				   double min_lat, double min_lon);

  static double km_per_deg_longitude;
  static double km_per_deg_latitude;
  static const double DEG_TO_RAD = 0.017453292519943295769236907684886; // PI/180
 private:
  // see http://andrew.hedges.name/experiments/haversine/
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


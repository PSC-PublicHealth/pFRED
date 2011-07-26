/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: Geo_Utils.cc
//

#include "Geo_Utils.h"

double Geo_Utils::km_per_deg_longitude = MEAN_US_KM_PER_DEG_LON;
double Geo_Utils::km_per_deg_latitude = MEAN_US_KM_PER_DEG_LAT;
// Geo_Utils::km_per_deg_longitude = Geo_Utils::ALLEG_KM_PER_DEG_LON;
// Geo_Utils::km_per_deg_latitude = Geo_Utils::ALLEG_KM_PER_DEG_LAT;

void Geo_Utils::set_km_per_degree(double lat) {
  lat *= Geo_Utils::DEG_TO_RAD;
  Geo_Utils::km_per_deg_longitude = cos(lat) * Geo_Utils::KM_PER_DEG_LAT;
  Geo_Utils::km_per_deg_latitude = Geo_Utils::KM_PER_DEG_LAT;
}

double Geo_Utils::haversine_distance (double lon1, double lat1, double lon2, double lat2) {
  // convert to radians
  lat1 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  double latH = sin(0.5 * (lat2 - lat1));
  latH *= latH;
  double lonH = sin(0.5 * (lon2 - lon1));
  lonH *= lonH;
  double a = latH + cos(lat1) * cos(lat2) * lonH;
  double c = 2 * atan2( sqrt(a), sqrt(1 - a));
  double dist = EARTH_RADIUS * c;
  return dist;
}

double Geo_Utils::spherical_cosine_distance (double lon1, double lat1, double lon2, double lat2) {
  // convert to radians
  lat1 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  return acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon2-lon1))*EARTH_RADIUS;
}


double Geo_Utils::spherical_projection_distance (double lon1, double lat1, double lon2, double lat2) {
  // convert to radians
  lat1 *= DEG_TO_RAD;
  lon1 *= DEG_TO_RAD;
  lat2 *= DEG_TO_RAD;
  lon2 *= DEG_TO_RAD;
  double dlat = (lat2-lat1);
  dlat *= dlat;
  double dlon = (lon2-lon1);
  double tmp = cos(0.5*(lat1+lat2))*dlon;
  tmp *= tmp;
  return EARTH_RADIUS*sqrt(dlat+tmp);
}

/*

int main () {
  double lon1 = -79;
  double lat1 = 40;
  int N = 0;
  for (double lon = -80.0; lon <= -79.0; lon += 0.01) {
    for (double lat = 40; lat < 40.1; lat += 0.01 ) {
      double d1 = haversine_distance(lon1,lat1,lon,lat);
      double d2 = spherical_cosine_distance(lon1,lat1,lon,lat);
      double d3 = spherical_projection_distance(lon1,lat1,lon,lat);
      N++;
      printf("%6.3f %5.2f %8.3f %8.3f %f\n", lon, lat, d1, d3, d1-d3);
    }
  }
  printf("N = %d\n", N);
}

*/


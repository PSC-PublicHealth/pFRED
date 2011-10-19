#ifndef _FRED_SEASONALITY_H
#define _FRED_SEASONALITY_H

#include "Abstract_Grid.h"
#include "Seasonality_Timestep_Map.h"
#include "Disease.h"
#include "Population.h"
#include "Global.h"
#include <vector>

class Abstract_Grid;

class Seasonality {

public:

  Seasonality(Abstract_Grid * grid);
  //~Seasonality(Abstract_Grid * grid);
  void update(int day);
  double get_seasonality_multiplier_by_lat_lon(double lat, double lon, int disease_id);
  double get_seasonality_multiplier_by_cartesian(double x, double y, int disease_id);
  double get_seasonality_multiplier(int row, int col, int disease_id);

  void print();
  void print_summary();

private:
  void update_seasonality_multiplier();
  double ** seasonality_values; // rectangular array of seasonality values
  vector < double ** > seasonality_multiplier; // result of applying each disease's seasonality/transmissibily kernel
  Seasonality_Timestep_Map * seasonality_timestep_map;
  Abstract_Grid * grid;

  struct point {
    int x, y;
    double value;
    point(int _x, int _y, double _value) {
      x = _x; y = _y; value = _value;
    }
  };

  void nearest_neighbor_interpolation(vector <point> points, double *** field);
  void print_field(double *** field);
  double get_average_seasonality_multiplier(int disease_id);
};

#endif // _FRED_SEASONALITY_H

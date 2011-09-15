//#ifndef _FRED_CLIMATE_H
//#define _FRED_CLIMATE_H

//#include "Abstract_Grid.h"
//#include "Climate_Timestep_Map.h"
//#include "Disease.h"
//#include "Population.h"
//#include "Global.h"
//#include <vector>

//class Abstract_Grid;

//class Climate {

//public:

//  Climate(Abstract_Grid * grid);
//  //~Climate(Abstract_Grid * grid);
//  void update(int day);
//  double get_climate_transmissibility_multiplier_by_lat_lon(double lat, double lon, int disease_id);
//  double get_climate_transmissibility_multiplier_by_cartesian(double x, double y, int disease_id);
//  double get_climate_transmissibility_multiplier(int row, int col, int disease_id);

//  void print();

//private:
//  void update_climate_transmissibility_multiplier();
//  double ** climate_values; // rectangular array of climate values
//  vector < double ** > climate_transmissibility_multiplier; // result of applying each disease's climate/transmissibily kernel
//  Climate_Timestep_Map * climate_timestep_map;
//  Abstract_Grid * grid;

//  struct point {
//    int x, y;
//    double value;
//    point(int _x, int _y, double _value) {
//      x = _x; y = _y; value = _value;
//    }
//  };

//  void nearest_neighbor_interpolation(vector <point> points, double *** field);
//  void print_field(double *** field);
//};

//#endif // _FRED_CLIMATE_H

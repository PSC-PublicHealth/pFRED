//#include "Climate.h"
//#include "Geo_Utils.h"
//#include "Random.h"
//#include "Timestep_Map.h"
//#include "Climate_Timestep_Map.h"
//#include "Disease.h"
//#include <vector>
//#include <iterator>


//Climate::Climate(Abstract_Grid * abstract_grid) {
//  grid = abstract_grid;
//  string param_name_str(Global::Climate_Timestep);
//  climate_timestep_map = new Climate_Timestep_Map(param_name_str);
//  climate_timestep_map->read_map();
//  climate_timestep_map->print();
//  climate_values = new double * [grid->get_rows()];
//  for (int i = 0; i < grid->get_rows(); i++) {
//    climate_values[i] = new double [grid->get_cols()];
//  }
//  for (int d = 0; d < Global::Diseases; d++) {
//    climate_transmissibility_multiplier.push_back(new double * [grid->get_rows()]);
//    for (int i = 0; i < grid->get_rows(); i++) {
//      climate_transmissibility_multiplier.back()[i] = new double [grid->get_cols()];
//    }
//  }
//}

//void Climate::update(int day) {
//  vector <point> points;
//  points.clear();
//  Climate_Timestep_Map::iterator it = climate_timestep_map->begin();
//  while (it != climate_timestep_map->end()) {
//    Climate_Timestep_Map::Climate_Timestep * cts = *it; 
//    if (cts->is_applicable(day, Global::Epidemic_offset)) {
//      double x,y;
//      Geo_Utils::translate_to_cartesian(cts->get_lat(),cts->get_lon(),
//          &x,&y,grid->get_min_lat(),grid->get_min_lon());
//      int row = grid->get_rows() - 1 - (int) (y/grid->get_grid_cell_size());
//      int col = (int) (x/grid->get_grid_cell_size());
//      points.push_back(point(row,col,cts->get_climate_value()));
//    }
//    it++;
//  }
//  nearest_neighbor_interpolation(points, &climate_values);
//  // store the cliamte modulated transmissibilities for all diseses
//  // so that we don't have to re-calculate this for every place that we visit
//  update_climate_transmissibility_multiplier();
//  if (Global::Verbose < 999) { print(); }
//}

//void Climate::update_climate_transmissibility_multiplier() {
//  for (int d = 0; d < Global::Diseases; d++) {
//    Disease * disease = Global::Pop.get_disease(d);
//    for (int r = 0; r < grid->get_rows(); r++) {
//      for (int c = 0; c < grid->get_cols(); c++) {
//        climate_transmissibility_multiplier[d][r][c] = disease->calculate_climate_transmissibility_multiplier(climate_values[r][c]);
//      }
//    }
//  }
//}

//double Climate::get_climate_transmissibility_multiplier_by_lat_lon(double lat, double lon, int disease_id) {
//  double x, y;
//  Geo_Utils::translate_to_cartesian(lat,lon,&x,&y,grid->get_min_lat(),grid->get_min_lon());
//  return get_climate_transmissibility_multiplier_by_cartesian(x,y,disease_id);
//}

//double Climate::get_climate_transmissibility_multiplier_by_cartesian(double x, double y, int disease_id) {
//  int row, col;
//  row = grid->get_rows()-1 - (int) (y/grid->get_grid_cell_size());
//  col = (int) (x/grid->get_grid_cell_size());
//  return get_climate_transmissibility_multiplier(row, col, disease_id);
//}

//double Climate::get_climate_transmissibility_multiplier(int row, int col, int disease_id) {
//  if ( row >= 0 && col >= 0 && row < grid->get_rows() && col < grid->get_cols() )
//    return climate_transmissibility_multiplier[disease_id][row][col];
//  else
//    return 0;
//}

//void Climate::nearest_neighbor_interpolation(vector <point> points, double *** field) {
//  int d1, d2, ties;
//  for (int r=0; r < grid->get_rows(); r++) {
//    for (int c=0; c < grid->get_cols(); c++) {
//      d1 = grid->get_rows() + grid->get_cols() + 1;
//      ties = 0;
//      for (vector <point>::iterator pit = points.begin(); pit != points.end(); pit++) {
//        d2 = abs( pit->x - r) + abs( pit->y - c );
//        if (d1 < d2) {
//          continue;
//        }
//        if (d1 == d2) {
//          if ((RANDOM()*((double)(ties+1)))>ties) {
//            (*field)[r][c] = pit->value;
//          }
//          ties++;
//        }
//        else {
//          (*field)[r][c] = pit->value; d1 = d2;
//        }
//      }
//    }
//  }
//}

//void Climate::print() {
//  cout << "Climate Values" << endl;
//  print_field(&climate_values);
//  cout << endl;
//  for (int d = 0; d < Global::Diseases; d++) {
//    printf("Climate Modululated Transmissibility for Disease[%d]\n",d);
//    print_field(&(climate_transmissibility_multiplier[d]));
//    cout << endl;
//  }
//}

//void Climate::print_field(double *** field) {
//  for (int r=0; r < grid->get_rows(); r++) {
//    for (int c=0; c < grid->get_cols(); c++) {
//      printf(" %4.4f", (*field)[r][c]);
//    }
//    cout << endl;
//  }
//}

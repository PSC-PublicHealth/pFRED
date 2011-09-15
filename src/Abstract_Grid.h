
#ifndef _ABSTRACT_GRID_H
#define _ABSTRACT_GRID_H

#include "Climate.h"

class Abstract_Grid {

public:

  int get_rows() { return rows; }
  int get_cols() { return cols; }
  double get_min_lat() { return min_lat; }
  double get_min_lon() { return min_lon; }
  double get_max_lat() { return max_lat; }
  double get_max_lon() { return max_lon; }
  double get_min_x() { return min_x; }
  double get_min_y() { return min_y; }
  double get_grid_cell_size() { return grid_cell_size; }
  


protected:

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

};

#endif

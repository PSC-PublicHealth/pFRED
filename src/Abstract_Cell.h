#ifndef _FRED_ABSTRACT_CELL_H
#define _FRED_ABSTRACT_CELL_H

#include <stdio.h>

class Abstract_Cell {

public:
  int get_row() { return row; }
  int get_col() { return col; }
  double get_min_x() { return min_x;}
  double get_max_x() { return max_x;}
  double get_min_y() { return min_y;}
  double get_max_y() { return max_y;}
  double get_center_y() { return center_y;}
  double get_center_x() { return center_x;}
  void print() { printf("cell %d %d: %f, %f, %f, %f\n",row,col,min_x,min_y,max_x,max_y);}

protected:
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
};

#endif

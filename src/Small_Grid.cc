/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Small_Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Small_Grid.h"
#include "Large_Grid.h"
#include "Small_Cell.h"
#include "Params.h"
#include "Random.h"

Small_Grid::Small_Grid(Large_Grid * lgrid) {
  large_grid = lgrid;
  int large_grid_rows = large_grid->get_rows();
  int large_grid_cols = large_grid->get_cols();
  int large_grid_cell_size = large_grid->get_grid_cell_size();
  min_lat = large_grid->get_min_lat();
  min_lon = large_grid->get_min_lon();
  max_lat = large_grid->get_max_lat();
  max_lon = large_grid->get_max_lon();
  min_x = large_grid->get_min_x();
  min_y = large_grid->get_min_y();
  max_x = large_grid->get_max_x();
  max_y = large_grid->get_max_y();

  get_parameters();

  // find the multiple to use in defining this grid;
  // the multiple must be an integer
  int mult = large_grid_cell_size / grid_cell_size;
  assert(mult == (1.0*large_grid_cell_size / (1.0* grid_cell_size)));

  rows = large_grid_rows * mult;
  cols = large_grid_cols * mult;

  if (Global::Verbose > 0) {
    fprintf(Global::Statusfp, "Small_Grid min_lon = %f\n", min_lon);
    fprintf(Global::Statusfp, "Small_Grid min_lat = %f\n", min_lat);
    fprintf(Global::Statusfp, "Small_Grid max_lon = %f\n", max_lon);
    fprintf(Global::Statusfp, "Small_Grid max_lat = %f\n", max_lat);
    fprintf(Global::Statusfp, "Small_Grid rows = %d  cols = %d\n",rows,cols);
    fprintf(Global::Statusfp, "Small_Grid min_x = %f  min_y = %f\n",min_x,min_y);
    fprintf(Global::Statusfp, "Small_Grid max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(Global::Statusfp);
  }

  grid = new Small_Cell * [rows];
  for (int i = 0; i < rows; i++) {
    grid[i] = new Small_Cell[cols];
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      grid[i][j].setup(this,i,j);
    }      
  }
}

void Small_Grid::get_parameters() {
  Params::get_param_from_string("grid_small_cell_size", &grid_cell_size);
}

Small_Cell * Small_Grid::get_grid_cell(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &grid[row][col];
  else
    return NULL;
}

Small_Cell * Small_Grid::get_grid_cell(fred::geo lat, fred::geo lon) {
  int row = get_row(lat);
  int col = get_col(lon);
  return get_grid_cell(row,col);
}

Small_Cell * Small_Grid::select_random_grid_cell() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &grid[row][col];
}

void Small_Grid::quality_control(char * directory) {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "small grid quality control check\n");
    fflush(Global::Statusfp);
  }
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      grid[row][col].quality_control();
    }
  }
  
  if (Global::Verbose>1) {
    char filename [256];
    sprintf(filename, "%s/smallgrid.dat", directory);
    FILE *fp = fopen(filename, "w");
    for (int row = 0; row < rows; row++) {
      if (row%2) {
	for (int col = cols-1; col >= 0; col--) {
	  double x = grid[row][col].get_center_x();
	  double y = grid[row][col].get_center_y();
	  fprintf(fp, "%f %f\n",x,y);
	}
      }
      else {
	for (int col = 0; col < cols; col++) {
	  double x = grid[row][col].get_center_x();
	  double y = grid[row][col].get_center_y();
	  fprintf(fp, "%f %f\n",x,y);
	}
      }
    }
    fclose(fp);
  }

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "small grid quality control finished\n");
    fflush(Global::Statusfp);
  }
}


// Specific to Small_Cell Small_Grid:



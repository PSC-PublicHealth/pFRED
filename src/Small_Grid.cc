/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Small_Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Place_List.h"
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
#if 1 //STB Added for cygwin, the assert for some reason is never true in 32-bit
      // Don't mess with unless you know what you are doing
  int mult = large_grid_cell_size / grid_cell_size;
  assert(mult == (1.0*large_grid_cell_size / (1.0* grid_cell_size)));
#else
  double mult_double = (1.0*double(large_grid_cell_size) / (1.0*double(grid_cell_size)));
  int mult = int(mult_double);
#endif
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
      // grid[i][j].setup(this,i,j);
      grid[i][j].setup(i, j, grid_cell_size, min_x, min_y);
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
    char filename [FRED_STRING_SIZE];
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


void Small_Grid::update(int day) {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      grid[row][col].update(day);
    }
  }
}


// Specific to Small_Cell Small_Grid:

void Small_Grid::initialize_gaia_data(char * directory, int run) {
  char gaia_dir[FRED_STRING_SIZE];

  // create GAIA data directory
  char gaia_top_dir[FRED_STRING_SIZE];
  sprintf(gaia_top_dir, "%s/GAIA", directory);
  Utils::fred_make_directory(gaia_top_dir);

  // create GAIA setup file
  char gaiafile[FRED_STRING_SIZE];
  sprintf(gaiafile, "%s/grid.txt", gaia_top_dir);
  FILE *fp = fopen(gaiafile, "w");
  fprintf(fp, "rows = %d\n", rows);
  fprintf(fp, "cols = %d\n", cols);
  fprintf(fp, "min_lat = %f\n", Global::Large_Cells->get_min_lat());
  fprintf(fp, "min_lon = %f\n", Global::Large_Cells->get_min_lon());
  fprintf(fp, "cell_x_size = %f\n", Geo_Utils::x_to_degree_longitude(grid_cell_size));
  fprintf(fp, "cell_y_size = %f\n", Geo_Utils::y_to_degree_latitude(grid_cell_size));
  fclose(fp);

  // make directory for this run
  sprintf(gaia_top_dir, "%s/run%d", gaia_top_dir, run);
  Utils::fred_make_directory(gaia_top_dir);

  // create GAIA sub directories for diseases and output vars
  for (int d = 0; d < Global::Diseases; d++) {
    char gaia_dis_dir[FRED_STRING_SIZE];
    sprintf(gaia_dis_dir, "%s/dis%d", gaia_top_dir, d);
    Utils::fred_make_directory(gaia_dis_dir);

    // create directories for specific output variables
    sprintf(gaia_dir, "%s/I", gaia_dis_dir);
    Utils::fred_make_directory(gaia_dir);
    sprintf(gaia_dir, "%s/Is", gaia_dis_dir);
    Utils::fred_make_directory(gaia_dir);
    sprintf(gaia_dir, "%s/C", gaia_dis_dir);
    Utils::fred_make_directory(gaia_dir);
    sprintf(gaia_dir, "%s/Cs", gaia_dis_dir);
    Utils::fred_make_directory(gaia_dir);
  }
}

void Small_Grid::print_gaia_data(char * directory, int run, int day) {
  for (int disease_id = 0; disease_id < Global::Diseases; disease_id++) {
    char dir[FRED_STRING_SIZE];
    sprintf(dir, "%s/GAIA/run%d", directory, run);
    print_output_data(dir, disease_id, Global::OUTPUT_I, (char *) "I", day);
    print_output_data(dir, disease_id, Global::OUTPUT_Is, (char *)"Is", day);
    print_output_data(dir, disease_id, Global::OUTPUT_C, (char *)"C", day);
    print_output_data(dir, disease_id, Global::OUTPUT_Cs, (char *)"Cs", day);
    print_population_data(dir, disease_id, day);
  }
}

void Small_Grid::print_population_data(char * dir, int disease_id, int day) {
  char filename[FRED_STRING_SIZE];
  printf("Printing popuation\n");
  Global::Places.get_cell_data_from_households(disease_id,1);
  sprintf(filename,"%s/dis%d/N/day-%d.txt",dir,disease_id,day);
  FILE *fp = fopen(filename, "w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) { 
      Small_Cell * cell = (Small_Cell *) &grid[i][j];
      int popsize = cell->get_popsize();
      if (popsize > 0){
	fprintf(fp, "%d %d %d\n", i, j, popsize);
      }
    }
  }
  fclose(fp);
}

void Small_Grid::print_output_data(char * dir, int disease_id, int output_code, char * output_str, int day) {
  Global::Places.get_cell_data_from_households(disease_id, output_code);
  char filename[FRED_STRING_SIZE];
  sprintf(filename, "%s/dis%d/%s/day-%d.txt", dir, disease_id, output_str, day);
  FILE *fp = fopen(filename, "w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Small_Cell * cell = (Small_Cell *) &grid[i][j];
      int count = cell->get_count();
      if (count > 0) {
	int popsize = cell->get_popsize();
	fprintf(fp, "%d %d %d %d\n", i, j, count, popsize);
      }
    }
  }
  fclose(fp);
}


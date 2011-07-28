/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Geo_Utils.h"
#include "Grid.h"
#include "Cell.h"
#include "Place_List.h"
#include "Place.h"
#include "Params.h"
#include "Random.h"
#include "Utils.h"
#include "Household.h"
#include "Population.h"
#include "Date.h"

Grid::Grid(double minlon, double minlat, double maxlon, double maxlat) {
  min_lon  = minlon;
  min_lat  = minlat;
  max_lon  = maxlon;
  max_lat  = maxlat;
  printf("min_lon = %f\n", min_lon);
  printf("min_lat = %f\n", min_lat);
  printf("max_lon = %f\n", max_lon);
  printf("max_lat = %f\n", max_lat);
  fflush(stdout);

  get_parameters();
  min_x = 0.0;
  max_x = (max_lon-min_lon) * Geo_Utils::km_per_deg_longitude;
  min_y = 0.0;
  max_y = (max_lat-min_lat) * Geo_Utils::km_per_deg_latitude;
  rows = 1 + (int) (max_y/grid_cell_size);
  cols = 1 + (int) (max_x/grid_cell_size);
  if (Global::Verbose) {
    printf("rows = %d  cols = %d\n",rows,cols);
    printf("max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(stdout);
  }

  grid = new Cell * [rows];
  for (int i = 0; i < rows; i++) {
    grid[i] = new Cell[cols];
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      grid[i][j].setup(this,i,j,j*grid_cell_size,(j+1)*grid_cell_size,
		       (rows-i-1)*grid_cell_size,(rows-i)*grid_cell_size);
    }
  }

  if (Global::Verbose > 1) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	printf("print grid[%d][%d]:\n",i,j);
	grid[i][j].print();
      }
    }
  }
  vacant_houses.clear();
}

void Grid::get_parameters() {
  get_param((char *) "grid_cell_size", &grid_cell_size);
}

Cell ** Grid::get_neighbors(int row, int col) {
  Cell ** neighbors = new Cell*[9];
  int n = 0;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      neighbors[n++] = get_grid_cell(i,j);
    }
  }
  return neighbors;
}


Cell * Grid::get_grid_cell(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &grid[row][col];
  else
    return NULL;
}


Cell * Grid::select_random_grid_cell() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &grid[row][col];
}


Cell * Grid::get_grid_cell_from_cartesian(double x, double y) {
  int row, col;
  row = rows-1 - (int) (y/grid_cell_size);
  col = (int) (x/grid_cell_size);
  // printf("x = %f y = %f, row = %d col = %d\n",x,y,row,col);
  return get_grid_cell(row, col);
}


Cell * Grid::get_grid_cell_from_lat_lon(double lat, double lon) {
  double x, y;
  translate_to_cartesian(lat,lon,&x,&y);
  return get_grid_cell_from_cartesian(x,y);
}


void Grid::translate_to_cartesian(double lat, double lon, double *x, double *y) {
  *x = (lon - min_lon) * Geo_Utils::km_per_deg_longitude;
  *y = (lat - min_lat) * Geo_Utils::km_per_deg_latitude;
}


void Grid::translate_to_lat_lon(double x, double y, double *lat, double *lon) {
  *lon = min_lon + x * Geo_Utils::km_per_deg_longitude;
  *lat = min_lat + y * Geo_Utils::km_per_deg_latitude;
}


void Grid::quality_control() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control check\n");
    fflush(Global::Statusfp);
  }
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      grid[row][col].quality_control();
    }
  }
  
  FILE *fp;
  fp = fopen("grid.dat", "w");
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
  
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "grid quality control finished\n");
    fflush(Global::Statusfp);
  }
}


// Specific to Cell Grid:

void Grid::record_favorite_places() {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      Cell * cell = (Cell *) &grid[row][col];
      cell->record_favorite_places();
      target_popsize += cell->get_neighborhood()->get_size();
      target_households += cell->get_houses();
    }
  }
}
/**
 * @brief Get all people living within a specified radius of a point.
 *
 * Finds patches overlapping radius_in_km from point, then finds all housholds in those patches radius_in_km distance from point.
 * Returns list of people in those housholds radius_in_km from point.
 * Used in Epidemic::update for geographical seeding.
 *
 * @author Jay DePasse
 */


vector < Place * >  Grid::get_households_by_distance(double lat, double lon, double radius_in_km) {
  double px, py;
  translate_to_cartesian(lat, lon, &px, &py);
  //  get cells around the point, make sure their rows & cols are in bounds
  int r1 = rows-1 - (int) ( ( py + radius_in_km ) / grid_cell_size );
  r1 = ( r1 >= 0 ) ? r1 : 0;
  int r2 = rows-1 - (int) ( ( py - radius_in_km ) / grid_cell_size );
  r2 = ( r2 <= rows-1 ) ? r2 : rows-1;

  int c1 = (int) ( ( px - radius_in_km ) / grid_cell_size );
  c1 = ( c1 >= 0 ) ? c1 : 0;
  int c2 = (int) ( ( px + radius_in_km ) / grid_cell_size );
  c2 = ( c2 <= cols-1 ) ? c2 : cols-1;

  printf("DEBUG: r1 %d r2 %d c1 %d c2 %d\n", r1,r2,c1,c2);

  vector <Place *> households; // store all households in cells ovelapping the radius

  for (int r = r1; r <= r2; r++ ) {
    for (int c = c1; c <= c2; c++ ) {
      Cell * p = get_grid_cell(r,c);
      vector <Place *> h = p->get_households();
      for (vector <Place *>::iterator hi = h.begin(); hi != h.end(); hi++) { 
        double hlat = (*hi)->get_latitude();
        double hlon = (*hi)->get_longitude();
        
        printf("DEBUG: household_latitude %f, household_longitude %f\n",hlat,hlon);
        double hx, hy;
        translate_to_cartesian(hlat, hlon, &hx, &hy);
        if (sqrt((px-hx)*(px-hx)+(py-hy)*(py-hy)) <= radius_in_km) {
          households.push_back((*hi));
        }
      }
    }
  }
  return households;
}


void Grid::add_vacant_house(Place * house) {
  vacant_houses.push_back(house);
}

Place * Grid::get_vacant_house() {
  Place * house = NULL;
  int count = vacant_houses.size();
  if (count > 0) {
    // pick a random vacant house
    int i = IRAND(0, count-1);
    house = vacant_houses[i];
    vacant_houses[i] = vacant_houses.back();
    vacant_houses.pop_back();
  }
  return house;
}

void Grid::population_migration() {
  select_emigrants();
  select_immigrants();
}


void Grid::select_emigrants() {
  Person * resident[100];
  if (Global::Verbose) {
    printf("EMIG oldpopsize = %d\n", Global::Pop.get_pop_size());
  }
  double epct;
  // monthly prob of emigrating
  epct = 0.02/12.0;
  int houses_to_emigrate;
  houses_to_emigrate = epct * target_households;
  printf("houses_to_emigrate = %d\n", houses_to_emigrate); fflush(stdout);
  int houses_vacated = 0;
  int people_removed = 0;
  for (int h = 0; h < houses_to_emigrate; h++) {

    Person * per = Global::Pop.select_random_person();
    if (RANDOM() < 0.5) {
      // pick a random person between 50 and 60
      int age = per->get_age();
      while (age < 50 || age > 60) {
	per = Global::Pop.select_random_person();
	age = per->get_age();
      }
    }
    Household * house_to_vacate = (Household *) per->get_household();

    // vacate the house
    int size = house_to_vacate->get_size();
    printf("house_to_vacate = %d  size = %d\n", house_to_vacate->get_id(), size); fflush(stdout);

    // get a list of all housemates:
    for (int i = 0; i < size; i++) { resident[i] = house_to_vacate->get_housemate(i); }

    for (int i = 0; i < size; i++) {
      Person * emigrant = resident[i];
      // unenroll for all favorite places, including the house
      printf("person_to_emigrate = %d  age = %d\n", emigrant->get_id(), emigrant->get_age()); fflush(stdout);
      printf("deleting from population\n"); fflush(stdout);
      // remove from population after withdrawing from activities
      Global::Pop.delete_person(emigrant);
      printf("deleted from population\n"); fflush(stdout);
      people_removed++;
    }
    houses_vacated++;
  }
  if (Global::Verbose) {
    printf("newpopsize = %d  people_removed = %d houses_vacated = %d\n",
	   Global::Pop.get_pop_size(), people_removed, houses_vacated);
    fflush(stdout);
  }
}

void Grid::select_immigrants() {
  int current_year = Global::Sim_Date->get_year();
  int current_popsize = Global::Pop.get_pop_size();
  printf("IMM curr = %d target = %d ", current_popsize, target_popsize);
  printf("vacant houses = %d  current_year = %d\n", get_vacant_houses(), current_year);
  int n = 0;
  int houses_filled = 0;
  while (current_popsize < target_popsize && get_vacant_houses() > 0) {
    // pick a vacant house
    Place * vacant = get_vacant_house();
    printf("vacant house = %d\n", vacant->get_id());
    // find its grid_cell
    Cell * p = vacant->get_grid_cell();
    printf("Cell row = %d col = %d\n", p->get_row(), p->get_col());
    // pick a random household from the grid_cell
    Household * clone_house = (Household *) p->select_random_household();
    int size = clone_house->get_orig_size();
    printf("IMM: clone house = %d size = %d\n", clone_house->get_id(), size);
    houses_filled++;
    // clone the original residents of this house
    for (int i = 0; i < size; i++) {
      // clone the ith original housemate
      int idx = clone_house->get_orig_id(i);
      char pstring[256];
      strcpy(pstring, Global::Pop.get_pstring(idx));

      int next_id = Global::Pop.get_next_id();
      int age, married, occ;
      char label[32], house[32], school[32], work[32];
      char sex;
      sscanf(pstring, "%s %d %c %d %d %s %s %s",
	     label, &age, &sex, &married, &occ, house, school, work);

      // make younger to reflect age based on next decennial
      int year_diff = 2010-current_year;
      if (age >= year_diff) age = age - year_diff;

      // redirect to the vacant house
      strcpy(house, vacant->get_label());

      // create clone
      Person * clone = new Person(next_id, age, sex, married, occ, house, school, work, &Global::Pop, Global::Sim_Date);

      // add to the popualtion
      Global::Pop.add_person(clone);

      clone->print(stdout,0);
      current_popsize++;
      n++;
    }
  }
  if (houses_filled > 0) {
    printf("IMM: %d house filled, %d people added, new popsize = %d = %d\n",
	   houses_filled, n, current_popsize, Global::Pop.get_pop_size());
    fflush(stdout);
  }
}

void Grid::print_household_distribution(char * dir, char * date_string, int run) {
  FILE *fp;
  int targ, count;
  double pct;
  char filename[256];
  sprintf(filename, "%s/household_dist_%s.%02d", dir, date_string, run);
  printf("print_household_dist entered, filename = %s\n", filename); fflush(stdout);

  fp = fopen(filename, "w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Cell * cell = (Cell *) &grid[i][j];
      count = cell->get_occupied_houses();
      targ = cell->get_target_households();
      if (targ > 0) pct = (100.0*count)/targ;
      else pct = 0.0;
      fprintf(fp, "%d %d %d %d %f\n", i, j, targ, count, pct);
    }
  }
  fclose(fp);
}


/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Grid.h
//

#ifndef _FRED_GRID_H
#define _FRED_GRID_H

#include <string.h>
#include "Place.h"
#include "Abstract_Grid.h"
class Large_Grid;
class Cell;
class Neighborhood;

class Grid : public Abstract_Grid {
public:

  /**
   * Set all of the attributes for the Grid
   *
   * @param minlon the minimum longitude for this Grid
   * @param minlat the minimum latitude for this Grid
   * @param maxlon the maximum longitude for this Grid
   * @param maxlat the maximum latitude for this Grid
   */
  Grid(Large_Grid * lgrid);
  Grid(fred::geo minlon, fred::geo minlat, fred::geo maxlon, fred::geo maxlat);
  ~Grid() {}

  void setup( Place::Allocator< Neighborhood > & neighborhood_allocator );

  /**
   * Get values from the parameter file
   */
  void get_parameters();

  /**
   * @param row the row where the Cell is located
   * @param col the column where the Cell is located
   * @return a pointer to the Cell at the row and column requested
   */
  Cell * get_grid_cell(int row, int col);
  Cell * get_grid_cell(fred::geo lat, fred::geo lon);
  Cell * select_random_grid_cell(double x0, double y0, double dist);
  Cell * select_random_neighbor(int row, int col);

  /**
   * @return a pointer to a random Cell in this Grid
   */
  Cell * select_random_grid_cell();

  /**
   * Used during debugging to verify that code is functioning properly.
   */
  void quality_control(char *directory);
  void quality_control(char *directory, double min_x, double min_y);

  /**
   * @brief Get all people living within a specified radius of a point.
   *
   * Finds patches overlapping radius_in_km from point, then finds all households in those patches radius_in_km distance from point.
   * Returns list of people in those households radius_in_km from point.
   * Used in Epidemic::update for geographical seeding.
   *
   * @author Jay DePasse
   * @param lat the latitude of the center of the circle
   * @param lon the longitude of the center of the circle
   * @param radius_in_km the distance from the center of the circle
   * @return a Vector of places
   */
  vector < Place * > get_households_by_distance(fred::geo lat, fred::geo lon, double radius_in_km);

  // Specific to Cell grid:
  /**
   * Make each Cell in this Grid store its favorite places, and then set target_popsize and target_households
   */
  void record_favorite_places();

  /**
   * Add an empty house to this Grid
   *
   * @param house the vacant house to add
   */
  void add_vacant_house(Place *house);

  /**
   * Return a pointer to an empty house in this Grid
   */
  Place * get_vacant_house();

  /**
   * @return the target_popsize
   */
  int get_target_popsize() { return target_popsize; }

  /**
   * @return the target_households
   */
  int get_target_households() { return target_households; }

  /**
   * @return the number of vacant houses in this grid
   */
  int get_vacant_houses() { return (int) vacant_houses.size(); }

  /**
   * Change the population dynamically
   *
   * @param day the simulation day
   */
  void population_migration(int day);

  /**
   * Write the household distributions to a file
   *
   * @param dir the directory where the file will go
   * @param date_string the date of the write (used for filename)
   * @param run the run id (used for filename)
   */
  void print_household_distribution(char * dir, char * date_string, int run);

protected:
  Cell ** grid;			      // Rectangular array of grid_cells
  Large_Grid * large_grid;	    // Pointer to surrounding large grid

  // Specific to Cell grid:
  int target_popsize;
  int target_households;
  int target_pop_age[100];
  vector <Place *> vacant_houses;

private:

  /**
   * Set the emigrants for a given day
   *
   * @param day the simulation day
   */

  void select_emigrants(int day);

  /**
   * Set the immigrants for a given day
   *
   * @param day the simulation day
   */
  void select_immigrants(int day);
};

#endif // _FRED_GRID_H

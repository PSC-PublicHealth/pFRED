/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cell.h
//

#ifndef _FRED_CELL_H
#define _FRED_CELL_H

#include <vector>
#include <string.h>

#include "Place.h"
class Person;
class Grid;

class Cell {
public:

  /**
   * Default constructor
   */
  Cell() {}
  ~Cell() {}

  /**
   * Set all of the attributes for the Cell
   *
   * @param grd the Grid to which this cell belongs
   * @param i the row of this Cell in the Grid
   * @param j the column of this Cell in the Grid
   * @param xmin the minimum x value for this cell
   * @param xmax the maximum x value for this cell
   * @param ymin the minimum y value for this cell
   * @param ymax the minimum y value for this cell
   *
   */
  void setup(Grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax);

  /**
   * Print out information about this object
   */
  void print();

  /**
   * Print out the x and y of this Cell as an ordered pair
   */
  void print_coord();

  /**
   * Used during debugging to verify that code is functioning properly.
   *
   */
  void quality_control();

  /**
   * @return the min_y
   */
  double get_min_y() { return min_y;}

  /**
   * @return the min_x
   */
  double get_min_x() { return min_x;}

  /**
   * @return the max_y
   */
  double get_max_y() { return max_y;}

  /**
   * @return the max_x
   */
  double get_max_x() { return max_x;}

  /**
   * @return the center_y
   */
  double get_center_y() { return center_y;}

  /**
   * @return the center_x
   */
  double get_center_x() { return center_x;}

  /**
   * Determines distance from this Cell to another.  Note, that it is distance from the
   * <strong>center</strong> of this Cell to the <strong>center</strong> of the Cell in question.
   *
   * @param the grid Cell to check against
   * @return the distance from this grid Cell to the one in question
   */
  double distance_to_grid_cell(Cell *grid_cell2);

  /**
   * @return the row
   */
  int get_row() { return row; }

  /**
   * @return the col
   */
  int get_col() { return col; }

  // specific to Cell grid:
  /**
   * Setup the neighborhood in this Cell
   */
  void make_neighborhood();

  /**
   * Add household to this Cell's household vector
   */
  void add_household(Place *p);

  /**
   * Create lists of persons, workplaces, schools (by age)
   */
  void record_favorite_places();

  /**
   * Select either this neighborhood or a neighborhood taken from one of this Cell's neighboring Cells in the Grid.
   *
   * @return a pointer to a Neighborhood
   */
  Place * select_neighborhood();

  /**
   * @return a pointer to a random Person in this Cell
   */
  Person * select_random_person();

  /**
   * @return a pointer to a random Household in this Cell
   */
  Place * select_random_household();

  /**
   * @return a pointer to a random Workplace in this Cell
   */
  Place * select_random_workplace();

  /**
   * @return a pointer to a random School in this Cell
   */
  Place * select_random_school(int age);

  /**
   * @return a pointer to a random Person in this Cell
   */
  Person * select_random_person_from_neighbors();

  /**
   * @return a count of houses in this Cell
   */
  int get_houses() { return houses;}

  /**
   * @return list of households in this grid cell.
   */
  vector <Place *> get_households() { return household; }

  /**
   * @return a pointer to this Cell's Neighborhood
   */
  Place * get_neighborhood() { return neighborhood; }

  /**
   * Add a person to this Cell's Neighborhood
   * @param per a pointer to the Person to add
   */
  void enroll(Person *per) { neighborhood->enroll(per); }

  /**
   * @return the count of occupied houses in the Cell
   */
  int get_occupied_houses() { return occupied_houses; }

  /**
   * Increment the count of occupied houses in the Cell
   */
  void add_occupied_house() { occupied_houses++; }

  /**
   * Decrement the count of occupied houses in the Cell
   */
  void subtract_occupied_house() { occupied_houses--; }

  /**
   * @return the target_households
   */
  int get_target_households() { return target_households; }

  /**
   * @return the target_popsize
   */
  int get_target_popsize() { return target_popsize; }

protected:

  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
  Cell ** neighbor_cells;
  Grid * grid;

  int houses;
  Place * neighborhood;
  vector <Place *> household;
  vector <Place *> school[20];
  vector <Place *> workplace;
  vector <Person *> person;

  // target grid_cell variables;
  int target_households;
  int target_popsize;
  int occupied_houses;
};

#endif // _FRED_CELL_H
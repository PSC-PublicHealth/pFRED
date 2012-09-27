/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Large_Cell.h
//

#ifndef _FRED_LARGE_CELL_H
#define _FRED_LARGE_CELL_H

#include <vector>
#include "Person.h"
#include "Abstract_Cell.h"
class Large_Grid;

class Large_Cell : public Abstract_Cell {
public:
  Large_Cell(Large_Grid * grd, int i, int j);
  Large_Cell() {}
  ~Large_Cell() {}
  void setup(Large_Grid * grd, int i, int j);
  void quality_control();
  double distance_to_grid_cell(Large_Cell *grid_cell2);
  void add_person(Person *p) { person.push_back(p); popsize++; }
  int get_popsize() { return popsize; }
  Person * select_random_person();
  void set_max_popsize(int n);
  int get_max_popsize() { return max_popsize; }
  double get_pop_density() { return pop_density; }
  void unenroll(Person *per);
  void add_workplace(Place *workplace);
  Place *get_workplace_near_to_school(Place *school, double * min_dist);
  Place * get_closest_workplace(double x, double y, int min_size, int max_size, double * min_dist);

protected:
  Large_Grid * grid;
  int popsize;
  vector <Person *> person;
  int max_popsize;
  double pop_density;
  vector <Place *> workplaces;
};

#endif // _FRED_LARGE_CELL_H

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
// File: Large_Cell.h
//

#ifndef _FRED_LARGE_CELL_H
#define _FRED_LARGE_CELL_H

#include <vector>

#include "Person.h"
#include "Abstract_Cell.h"
#include "Global.h"
#include "Utils.h"

class Large_Grid;



class Large_Cell : public Abstract_Cell {
public:
  Large_Cell(Large_Grid * grd, int i, int j);
  Large_Cell() {}
  ~Large_Cell() {}
  void setup(Large_Grid * grd, int i, int j);
  void quality_control();
  double distance_to_grid_cell(Large_Cell *grid_cell2);
  void add_person( Person * p ) {
    // <-------------------------------------------------------------- Mutex
    fred::Scoped_Lock lock(mutex);
    person.push_back( p );
    ++demes[ p->get_deme_id() ];
    ++popsize;
  }
  int get_popsize() { return popsize; }
  Person * select_random_person();
  void set_max_popsize(int n);
  int get_max_popsize() { return max_popsize; }
  double get_pop_density() { return pop_density; }
  void unenroll(Person *per);
  void add_workplace(Place *workplace);
  Place *get_workplace_near_to_school(Place *school);
  Place * get_closest_workplace(double x, double y, int min_size, int max_size, double * min_dist);

  int get_id() { return id; }

  unsigned char get_deme_id();

protected:
  fred::Mutex mutex;
  Large_Grid * grid;
  int popsize;
  vector <Person *> person;
  int max_popsize;
  double pop_density;
  int id;
  static int next_cell_id;
  vector <Place *> workplaces;
  std::map< unsigned char, int > demes;
};

#endif // _FRED_LARGE_CELL_H

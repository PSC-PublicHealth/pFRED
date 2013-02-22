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
// File: Small_Cell.h
//

#ifndef _FRED_SMALL_CELL_H
#define _FRED_SMALL_CELL_H

#include "Abstract_Cell.h"

class Small_Grid;

class Small_Cell : public Abstract_Cell {
public:
  Small_Cell() {}
  ~Small_Cell() {}
  void setup(Small_Grid * grd, int i, int j);
  void quality_control();
  void update(int day) { cases = 0; popsize = 0; }
  double distance_to_grid_cell(Small_Cell *grid_cell2);
  int get_cases() { return cases; }
  int get_popsize() { return popsize; }
  void print();
  void household_report(int inf, int total) { cases += inf; popsize += total; print();} 

protected:
  Small_Grid * grid;
  int cases;
  int popsize;
};

#endif // _FRED_SMALL_CELL_H

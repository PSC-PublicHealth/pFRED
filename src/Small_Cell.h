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
  double distance_to_grid_cell(Small_Cell *grid_cell2);

protected:
  Small_Grid * grid;
};

#endif // _FRED_SMALL_CELL_H

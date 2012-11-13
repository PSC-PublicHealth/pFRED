/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/


#include "EvolutionFactory.h"
#include "Evolution.h"
#include "MSEvolution.h"
#include "FergEvolution.h"
//#include "BFEvolution.h"
#include "Global.h"
#include "Utils.h"

Evolution *EvolutionFactory :: newEvolution(int type) {
  switch(type) {
    case 0:
      return new Evolution;
    case 1:
      return new MSEvolution;
    case 2:
      return new FergEvolution;
    //case 3:
    //  return new BFEvolution;
    default:
      FRED_WARNING("Unknown Evolution type (%d) supplied to EvolutionFactory.  Using the default.", type);
      return new Evolution;
    }
  }


#include "EvolutionFactory.h"
#include "Evolution.h"
#include "Global.h"
#include "Utils.h"

Evolution *EvolutionFactory :: newEvolution(int type) {
  switch(type) {
    case 0:
      return new Evolution;
    default:
      Utils::fred_warning("Unknown Evolution type (%d) supplied to EvolutionFactory.  Using the default.", type);
      return new Evolution;
    }
  }

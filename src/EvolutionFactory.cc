
#include "EvolutionFactory.h"
#include "Evolution.h"
#include "Global.h"

Evolution *EvolutionFactory :: newEvolution(int type) {
  switch(type) {
    case 0:
      return new Evolution;
      // default - TODO print error msg
    }
  }

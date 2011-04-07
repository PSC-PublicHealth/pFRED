
#include "EvolutionFactory.h"
#include "Evolution.h"
#include "DRI_Evolution.h"
#include "Global.h"

Evolution *EvolutionFactory :: newEvolution(int type) {
  switch(type) {
    case 0:
      return new Evolution;
    case 1:
      return new DRI_Evolution;

    // default - TODO print error msg
  }
}

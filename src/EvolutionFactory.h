
#ifndef _FRED_EVOLUTION_FACTORY_H
#define _FRED_EVOLUTION_FACTORY_H

#include <map>

class Evolution;

class EvolutionFactory {
public:
  static Evolution *newEvolution(int type);
};

#endif



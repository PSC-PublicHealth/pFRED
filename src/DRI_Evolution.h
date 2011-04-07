#ifndef _FRED_DRI_EVOLUTION_H
#define _FRED_DRI_EVOLUTION_H

#include <map>
#include "Evolution.h"

class Infection;
class AV_Health;
class Antiviral;
class Infection;
class Health;

using namespace std;

class DRI_Evolution : public Evolution 
{
 public:

  void reset(int run);
  void doEvolution(Infection *infection, map<int, double> *loads);
  void avEffect(Antiviral *av, Health *health, int disease, int cur_day, AV_Health* av_health);
  void print();

 private:
  static const int RESISTANT_STRAIN = 1; // id of resistant strain
  static const int NON_RESISTANT_STRAIN = 0;
  double mutationProb;
  double t, p;

  int numRes, numNonRes, numMutations, numTot;
};

#endif


#ifndef _FRED_EVOLUTION_H
#define _FRED_EVOLUTION_H

#include <map>

class Infection;
class AV_Health;
class Antiviral;
class Infection;
class Health;

class Evolution {
public:

  /**
   * Put this object back to its original state
   */
  virtual void reset(int reset);

  /**
   * @param infection a pointer to an Infection object
   * @param loads a pointer to a map of viral loads
   */
  virtual void doEvolution(Infection *infection, std::map<int, double> *loads);

  /**
   * @param av a pointer to an Antiviral object
   * @param health a pointer to a Health object
   * @param disease the disease to which this applies
   * @param cur_day the simulation day
   * @param av_health a pointer to a particular AV_Health object
   */
  virtual void avEffect(Antiviral *av, Health *health, int disease, int cur_day, AV_Health *av_health);

  /**
   * @param the simulation day
   * @return a pointer to a map of Primary Loads for a given day
   */
  std::map<int, double> * getPrimaryLoads(int day);

  /**
   * @param the simulation day
   * @param the particular strain of the disease
   * @return a pointer to a map of Primary Loads for a particular strain of the disease on a given day
   */
  std::map<int, double> * getPrimaryLoads(int day, int strain);

  /**
   * Print out information about this object
   */
  virtual void print();
};

#endif



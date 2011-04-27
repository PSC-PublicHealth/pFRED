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
    static Evolution *newEvolution(int type);

    virtual void reset(int reset);
    virtual void doEvolution(Infection *infection, std :: map<int, double> *loads);
    virtual void avEffect(Antiviral *av, Health *health, int disease, int cur_day, AV_Health *av_health);
    std::map<int, double> *getPrimaryLoads(int day);
    virtual void print();
  };

#endif



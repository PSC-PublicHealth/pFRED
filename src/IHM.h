#ifndef _FRED_IHM_H
#define _FRED_IHM_H

#include <vector>
#include <map>

class Infection;
class Trajectory;
class Disease;

class IHM {
  public:
    static IHM *newIHM(int type);

    virtual void setup(Disease *dis);
		virtual Trajectory *getTrajectory(Infection *infection, std::map<int, double> *loads) = 0; 
    int get_max_days() {return max_days; }
  protected:
    Disease *disease;
    int max_days;
};

#endif

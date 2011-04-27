#ifndef _FRED_IntraHost_H
#define _FRED_IntraHost_H

#include <vector>
#include <map>

class Infection;
class Trajectory;
class Disease;

class IntraHost {
  public:
    static IntraHost *newIntraHost(int type);
    virtual void setup(Disease *dis);
    virtual Trajectory *getTrajectory(Infection *infection, std::map<int, double> *loads) = 0;
    int get_max_days() {
      return max_days;
      }
    virtual int get_days_symp();
  protected:
    Disease *disease;
    int max_days;
  };

#endif

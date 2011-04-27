#ifndef _FRED_FIXED_IntraHost_H
#define _FRED_FIXED_IntraHost_H

#include <vector>
#include <map>

#include "IntraHost.h"
#include "Infection.h"
#include "Trajectory.h"

class Infection;
class Trajectory;

class FixedIntraHost : public IntraHost {
  public:
    Trajectory *getTrajectory(Infection *infection, std :: map<int, double> *loads);
    void setup(Disease *disease);
    virtual int get_days_symp();

  private:
    // library of trajectories and cdf over them
    std :: vector< std :: vector<double> > infLibrary;
    std :: vector< std :: vector<double> > sympLibrary;
    std :: vector<double> probabilities;
  };

#endif

#ifndef _FRED_FIXED_IHM_H
#define _FRED_FIXED_IHM_H

#include <vector>
#include <map>

#include "IHM.h"
#include "Infection.h"
#include "Trajectory.h"

class Infection;
class Trajectory;

class FixedIHM : public IHM {
	public:
		Trajectory *getTrajectory(Infection *infection, std :: map<int, double> *loads);
	  void setup(Disease *disease);

	private:
    // library of trajectories and cdf over them
    std :: vector< std :: vector<double> > infLibrary;
    std :: vector< std :: vector<double> > sympLibrary;
    std :: vector<double> probabilities;
};

#endif

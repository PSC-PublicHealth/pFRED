#ifndef _FRED_AEB_H
#define _FRED_AEB_H

#include <map>
#include <vector>

#include "IHM.h"

class Infection;
class Trajectory;

using namespace std;

class AEB : public IHM {
  // TODO set params from params file
  // TODO set all initial values
  
  public:
    Trajectory *getTrajectory(Infection *infection, map<int, double> *loads);
    void setup(Disease *disease);

  private:
    double get_inoculum_particles (double infector_particles); 
    vector<double> getInfectivities(double *viralTiter, int duration);
    vector<double> getSymptomaticity(double *interferon, int duration);

    static const int MAX_LENGTH = 10;
    
    double viral_titer_scaling;
    double viral_titer_latent_threshold;
    double interferon_scaling;
    double interferon_threshold;
};


#endif

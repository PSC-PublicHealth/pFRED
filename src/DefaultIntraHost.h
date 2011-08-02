#ifndef _FRED_DEFAULT_IntraHost_H
#define _FRED_DEFAULT_IntraHost_H

#include <vector>
#include <map>

#include "IntraHost.h"
#include "Infection.h"
#include "Trajectory.h"

class Infection;
class Trajectory;

class DefaultIntraHost : public IntraHost {
    // TODO Move reqd stuff from disease to here
  public:
    DefaultIntraHost();
    ~DefaultIntraHost();
    Trajectory *getTrajectory(Infection *infection, std :: map<int, double> *loads);
    void setup(Disease *disease);
    int get_days_latent();
    int get_days_incubating();
    int get_days_asymp();
    int get_days_symp();
    int get_days_susceptible();

    int get_symptoms();
    double get_asymp_infectivity() {
      return asymp_infectivity;
      }
    double get_symp_infectivity() {
      return symp_infectivity;
      }
    int get_max_days() {
      return max_days;
      }
    double get_prob_symptomatic() {
      return prob_symptomatic;
      }

    int get_infection_model() {
      return infection_model;
      }

  private:
    double asymp_infectivity;
    double symp_infectivity;
    int infection_model;
    int max_days_latent;
    int max_days_incubating;
    int max_days_asymp;
    int max_days_symp;
    int max_days;
    double *days_latent;
    double *days_incubating;
    double *days_asymp;
    double *days_symp;
    double prob_symptomatic;
  };

#endif


#include "Evolution.h"
#include "Infection.h"
#include "Trajectory.h"
#include "Infection.h"
#include "Global.h"
#include "AEB.h"
#include "Antiviral.h"
#include "Health.h"
#include "AV_Health.h"

#include <map>

using namespace std;

void Evolution :: doEvolution(Infection *infection, map<int, double> *loads)
{
  Disease * disease = infection->get_disease();
  Trajectory *trajectory = disease->getTrajectory(infection, loads);
  infection->setTrajectory(trajectory);
  infection->set_susceptibility_period(0);
}

void Evolution :: avEffect(Antiviral *av, Health *health, int disease, int cur_day, AV_Health* av_health){
  // If this is the first day of AV Course
  if(cur_day == av_health->get_av_start_day()) {
    av->modify_susceptiblilty(health, disease);

    // If you are already exposed, we need to modify your infection
    if((health->get_exposure_date(disease) > -1) && (cur_day > health->get_exposure_date(disease))){
      if(Debug > 3) cout << "reducing an already exposed person\n";
      av->modify_infectivity(health, disease);
      //av->modify_symptomaticity(health, disease, cur_day);
    }
  }

  // If today is the day you got exposed, prophilaxis
  if(cur_day == health->get_exposure_date(disease)) {
    if(Debug > 3) cout << "reducing agent on the day they are exposed\n";
    av->modify_infectivity(health, disease);
    av->modify_symptomaticity(health, disease, cur_day);
  }

  // If this is the last day of the course
  if(cur_day == av_health->get_av_end_day()) {
    if(Debug > 3) cout << "resetting agent to original state\n";
    av->modify_susceptiblilty(health, disease);
    if(cur_day >= health->get_exposure_date(disease)) {
      av->modify_infectivity(health, disease);
    }
  }

  // do evolutions...
}

void Evolution :: print() {}
void Evolution :: reset(int run) {}

map<int, double> *Evolution :: getPrimaryLoads(int day) {
  map<int, double> *loads = new map<int, double>;
  loads->insert( pair<int, double> (0, 1) );
  return loads;
}

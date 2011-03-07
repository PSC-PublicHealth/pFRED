
#include "DRI_Evolution.h"
#include "cmath"
#include "Evolution.h"
#include "Infection.h"
#include "Trajectory.h"
#include "Infection.h"
#include "Global.h"
#include "AEB.h"
#include "Antiviral.h"
#include "Health.h"
#include "AV_Health.h"
#include "Person.h"
#include <map>
#include "Params.h"

using namespace std;

void DRI_Evolution :: reset(int run)
{
  get_param((char *)"mut_prob", &p);
  get_param((char *)"rel_trans", &t);
  numRes = 0; numTot = 100; numNonRes = 0; numMutations = 0;// TODO 100 -- seeding
}

void DRI_Evolution :: print()
{
  printf("RESULTS: %d %d %d %lf %lf %lf %lf\n", numTot, numRes, numMutations, 1.0* numRes/numTot, 1.0 * (numRes + numMutations) / (numTot), t, p);
}
/*
void DRI_Evolution :: doEvolution(Infection *infection, map<int, double> *loads)
{
 for(map<int, double> :: iterator it = loads->begin(); it != loads->end(); it++) {
    cout<< "LOADS " << it->first << " " << it->second;
  }
  cout << endl;

  const int res = DRI_Evolution::RESISTANT_STRAIN, nres = DRI_Evolution::NON_RESISTANT_STRAIN;
  int strain = nres;
  if( loads->find(res) != loads->end() ) strain = res;

  double trans = infection->get_disease()->get_transmissibility(strain);

  //if(strain == res) trans *= t;

  double r = RANDOM();
  if(r < trans){
    Evolution::doEvolution(infection, loads);
      numRes++;
    //if(strain == res) numRes++;
    //else numNonRes++;
    numTot++;
  }
//  else{
//    Trajectory *trajectory = new Trajectory;
//    infection->setTrajectory(trajectory);
//  }
  
}
 */

void DRI_Evolution :: doEvolution(Infection *infection, map<int, double> *loads)
{
  if(loads->find(1) != loads->end()){
    printf("LOADS\n");
  }
  int strain = 0;
  double trans = infection->get_disease()->get_transmissibility(0);

  Evolution::doEvolution(infection, loads);
  if(infection->get_trajectory()->contains(1)) strain = 1, trans *= t;

  double r = RANDOM();
  if(r >= trans){
    Trajectory *trajectory = new Trajectory;
    infection->setTrajectory(trajectory);
  } 
  else {
    if(strain == 0) numNonRes++;
    else numRes++;
    numTot++;
  }
}

void DRI_Evolution :: avEffect(Antiviral *av, Health *health, int disease, int cur_day, AV_Health* av_health)
{
  const int res = DRI_Evolution::RESISTANT_STRAIN, nres = DRI_Evolution::NON_RESISTANT_STRAIN;

  Infection *infection = health->get_infection(disease);
  if(infection != NULL && ! av->is_prophylaxis()){
    Trajectory *trajectory = infection->get_trajectory();
    
    if(trajectory != NULL){
      if(trajectory->contains(res)) { // if infected by resistant strain, then drug has no effect
        return;
        //vector<double> infectivity = trajectory->get_infectivity_trajectory(res);
        //if(cur_day < infectivity.size() && infectivity[cur_day] != 0) 
        //    return;
      }
      // otherwise, mutate to resistant strain with some probability
        double r = RANDOM();
        mutationProb = 0.0 + p;

        //printf("RAND: %lf %lf\n", r, mutationProb);
        if(r < (mutationProb)) {
          vector<double> infectivity = trajectory->get_infectivity_trajectory(nres);
          map<int, vector<double> > infs;
          infs.insert( pair<int, vector<double> > (res, infectivity) );
          trajectory->set_infectivities(infs);
          infection->setTrajectory(trajectory);
          //trajectory->print();
          /*numRes++; numNonRes--;*/ 
          numMutations++;

          if(Verbose > 1) printf("Mutating non-resistant strain to resistant strain for person %d\n", health->get_self()->get_id());
        }
    }
  }

  // Call superclass method for normal behavior
  //Evolution::avEffect(av, health, disease, cur_day, av_health);
}


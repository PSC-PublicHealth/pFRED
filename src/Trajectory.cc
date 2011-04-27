#include "Trajectory.h"
#include <vector>
#include <iostream>
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include <string>
#include <sstream>

using namespace std;

Trajectory::Trajectory() {
  duration = 0;
  }

Trajectory::Trajectory(map< int, trajectory_t > infectivity_copy, trajectory_t symptomaticity_copy) {
  duration = 0;
  infectivity = infectivity_copy;
  symptomaticity = symptomaticity_copy;

  for (map< int, trajectory_t >::iterator strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
    if ( (int) strain_iterator->second.size() > duration) {
      duration = strain_iterator->second.size();
      }
    }

  if ( (int) symptomaticity.size() > duration) {
    duration = symptomaticity.size();
    }
  }

Trajectory * Trajectory::clone() {
  Trajectory * cloned_trajectory = new Trajectory(infectivity, symptomaticity);
  return cloned_trajectory;
  }

bool Trajectory::contains(int strain) {
  return ( infectivity.find(strain) != infectivity.end() );
  }

void Trajectory::set_infectivities(map<int, vector<double> > inf) {
  infectivity = inf;
  }

trajectory_t Trajectory::get_infectivity_trajectory(int strain) {
  return infectivity[strain];
  }

trajectory_t Trajectory::get_symptomaticity_trajectory() {
  return symptomaticity;
  }

void Trajectory::set_infectivity_trajectory(int strain, trajectory_t inf) {
  map< int, trajectory_t >::iterator it = infectivity.find(strain);

  if (it != infectivity.end()) {
    infectivity.erase(it);
    }

  infectivity[strain] = inf;

  if (duration < (int) infectivity[strain].size()) {
    duration = infectivity[strain].size();
    }
  }

void Trajectory::set_symptomaticity_trajectory(trajectory_t symt) {
  symptomaticity = symt;

  if (duration < (int) symptomaticity.size()) {
    duration = (int) symptomaticity.size();
    }
  }

Trajectory::point Trajectory::get_data_point(int t) {
  double point_infectivity = 0.0;
  double point_symptomaticity = 0.0;

  if (duration > t) {
    map< int, trajectory_t >::iterator strain_iterator;

    for (strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
      if ( (int) strain_iterator->second.size() > t) {
        point_infectivity += strain_iterator->second[t];
        }
      }

    if ( (int) symptomaticity.size() > t) {
      point_symptomaticity += symptomaticity[t];
      }
    }

  return Trajectory::point(point_infectivity, point_symptomaticity);
  }

void Trajectory::calculate_aggregate_infectivity() {
  // not currently used anywhere, could be used in the trajectory iterator
  aggregate_infectivity.assign(duration,0.0);
  map< int, trajectory_t >::iterator strain_iterator;
  trajectory_t::iterator strain_infectivity_iterator;

  for (strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
    int t = 0;

    for (strain_infectivity_iterator = strain_iterator->second.begin(); strain_infectivity_iterator != strain_iterator->second.end(); ++strain_infectivity_iterator) {
      cout << t << "  " << *strain_infectivity_iterator << "  " << duration << endl;
      aggregate_infectivity[t] += *strain_infectivity_iterator;
      t++;
      }
    }
  }

string Trajectory::to_string() {
  ostringstream os;
  os << "Infection Trajectories:";
  map< int, trajectory_t >::iterator map;
  trajectory_t::iterator vec;

  for (map = infectivity.begin(); map != infectivity.end(); ++map) {
    os << endl << " Strain " << map->first << ":";

    for (vec = map->second.begin(); vec != map->second.end(); ++vec) {
      os << " " << *vec;
      }
    }

  os << endl << "Symptomaticity Trajectories:" << endl;

  for (vec = symptomaticity.begin(); vec != symptomaticity.end(); ++vec) {
    os << " " << *vec;
    }

  return os.str();
  }

void Trajectory::print() {
  cout << to_string() << endl;
  }

map<int, double> *Trajectory::getCurrentLoads(int day) {
  map<int, double> *infectivities = new map<int, double>;

  map< int, trajectory_t > :: iterator it;

  for(it = infectivity.begin(); it != infectivity.end(); it++) {
    pair<int, double> p = pair<int, double> (it->first, (it->second)[day]);
    infectivities->insert(p);
    }

  return infectivities;
  }

map<int, double> *Trajectory::getInoculum(int day) {
  map<int, double> *infectivities = new map<int, double>;

  map< int, trajectory_t > :: iterator it;

  for(it = infectivity.begin(); it != infectivity.end(); it++) {
    pair<int, double> p = pair<int, double> (it->first, (it->second)[day]);
    infectivities->insert(p);
    }

  return infectivities;
  }

void Trajectory::modifySympPeriod(int startDate, int daysLeft) {
  // symptomaticty and infectivity trajectories become 0 after startDate + daysLeft
  int endDate = startDate + daysLeft;

  if(endDate > (int) symptomaticity.size()) return;

  symptomaticity.resize(endDate, 1);
  map< int, trajectory_t > :: iterator it;

  for(it = infectivity.begin(); it != infectivity.end(); it++) {
    (it->second).resize(endDate, 1);
    }
  }

void Trajectory::modifyAsympPeriod(int startDate, int daysLeft, int sympDate) {
  int endDate = startDate + daysLeft;

  // if decreasing the asymp period
  if(endDate < sympDate) {
    for(int i = startDate, j = sympDate ; i < (int) symptomaticity.size(); i++, j++) {
      symptomaticity[i] = symptomaticity[j];
      }

    symptomaticity.resize(symptomaticity.size() - sympDate + daysLeft, 0);
    map< int, trajectory_t > :: iterator it;

    for(it = infectivity.begin(); it != infectivity.end(); it++) {
      trajectory_t &inf = it->second;

      for(int i = startDate, j = sympDate ; i < (int) inf.size(); i++, j++) {
        inf[i] = inf[j];
        }

      inf.resize(symptomaticity.size() - sympDate + daysLeft, 0);
      }
    }
  // if increasing the asymp period
  else {
    int daysExtended = endDate - sympDate;
    trajectory_t :: iterator it = symptomaticity.begin();

    for(int i = 0; i < sympDate; i++) it++;

    symptomaticity.insert(it, daysExtended, 0.0);

    map< int, trajectory_t > :: iterator inf_it;

    for(inf_it = infectivity.begin(); inf_it != infectivity.end(); inf_it++) {
      // infectivity value for new period = infectivity just before becoming symptomatic
      //  == asymp_infectivity for FixedIntraHost
      trajectory_t &inf = inf_it->second;
      it = inf.begin();

      for(int i = 0; i < sympDate; i++) it++;

      inf.insert(it, daysExtended, inf[sympDate-1]);
      }
    }
  }

void Trajectory :: modifyDevelopsSymp(int sympDate, int sympPeriod) {
  int endDate = sympDate + sympPeriod;

  if(endDate < (int) symptomaticity.size()) {
    symptomaticity.resize(endDate);
    }
  else {
    symptomaticity.resize(endDate, 1);
    map< int, trajectory_t > :: iterator inf_it;

    for(inf_it = infectivity.begin(); inf_it != infectivity.end(); inf_it++) {
      trajectory_t &inf = inf_it->second;
      trajectory_t :: iterator it = inf.end();
      it--;
      inf.insert(it, endDate - symptomaticity.size(), inf[endDate-1]);
      }
    }
  }

void Trajectory :: getAllStrains(vector<int>& strains) {
  strains.clear();
  map< int, trajectory_t > :: iterator inf_it;

  for(inf_it = infectivity.begin(); inf_it != infectivity.end(); inf_it++) {
    strains.push_back(inf_it->first);
    }
  }





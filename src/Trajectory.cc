#include "Trajectory.h"
#include <vector>
#include <iostream>
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include <string>
#include <sstream>

Trajectory::Trajectory() {
	duration = 0;
}

Trajectory::Trajectory(std::map< int, std::vector<double> > infectivity_copy, std::vector<double> symptomaticity_copy) {
	duration = 0;
	infectivity = infectivity_copy;
	symptomaticity = symptomaticity_copy;
	for (std::map< int, std::vector<double> >::iterator strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
		if (strain_iterator->second.size() > duration) { duration = strain_iterator->second.size(); }
	}
	if (symptomaticity.size() > duration) { duration = symptomaticity.size(); }
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

std::vector<double> Trajectory::get_infectivity_trajectory(int strain) {
	return infectivity[strain];
}

std::vector<double> Trajectory::get_symptomaticity_trajectory() {
	return symptomaticity;
}

void Trajectory::set_infectivity_trajectory(int strain, std::vector<double> inf) {
	std::map< int, std::vector<double> >::iterator it = infectivity.find(strain);
	if (it != infectivity.end()) {
		infectivity.erase(it);
	}
	infectivity[strain] = inf;
	if (duration < infectivity[strain].size()) { duration = infectivity[strain].size(); }
}

void Trajectory::set_symptomaticity_trajectory(std::vector<double> symt) {
	symptomaticity = symt;
	if (duration < symptomaticity.size()) { duration = symptomaticity.size(); }
}

Trajectory::point Trajectory::get_data_point(int t) {
	double point_infectivity = 0.0;
	double point_symptomaticity = 0.0;
	if (duration > t) {
		std::map< int, std::vector<double> >::iterator strain_iterator;
		for (strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
			if (strain_iterator->second.size() > t) { point_infectivity += strain_iterator->second[t]; }
		}
		if (symptomaticity.size() > t) {
			point_symptomaticity += symptomaticity[t];
		}
	}
	return Trajectory::point(point_infectivity, point_symptomaticity);
}

void Trajectory::calculate_aggregate_infectivity() {
	// not currently used anywhere, could be used in the trajectory iterator
	aggregate_infectivity.assign(duration,0.0);
	std::map< int, std::vector<double> >::iterator strain_iterator;
	std::vector<double>::iterator strain_infectivity_iterator;
	for (strain_iterator = infectivity.begin(); strain_iterator != infectivity.end(); ++strain_iterator) {
		int t = 0;
		for (strain_infectivity_iterator = strain_iterator->second.begin(); strain_infectivity_iterator != strain_iterator->second.end(); ++strain_infectivity_iterator) {
			cout << t << "  " << *strain_infectivity_iterator << "  " << duration << endl;
			aggregate_infectivity[t] += *strain_infectivity_iterator;
			t++;
		}
	}
}	

std::string Trajectory::to_string() {
	std::ostringstream os;
	os << "Infection Trajectories:";
	std::map< int, std::vector<double> >::iterator map;
	std::vector<double>::iterator vec;
	for (map = infectivity.begin(); map != infectivity.end(); ++map) {
		os << std::endl << " Strain " << map->first << ":";
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

std::map<int, double> *Trajectory::getCurrentLoads(int day){
  std::map<int, double> *infectivities = new std::map<int, double>;

  std::map< int, std::vector<double> > :: iterator it;
  for(it = infectivity.begin(); it != infectivity.end(); it++){
	  pair<int, double> p = pair<int, double> (it->first, (it->second)[day]);
	  infectivities->insert(p);
  }
  return infectivities;
}

std::map<int, double> *Trajectory::getInoculum(int day) {
  std::map<int, double> *infectivities = new std::map<int, double>;

  std::map< int, std::vector<double> > :: iterator it;
  for(it = infectivity.begin(); it != infectivity.end(); it++){
  	pair<int, double> p = pair<int, double> (it->first, (it->second)[day]);
	  infectivities->insert(p);
  }
  return infectivities;
}


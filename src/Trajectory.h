#ifndef _FRED_TRAJECTORY_H
#define _FRED_TRAJECTORY_H


#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <string>

class Trajectory {
	public:
		Trajectory();
		Trajectory(std::map< int, std::vector<double> > infectivity_copy, std::vector<double> symptomaticity_copy);
		Trajectory * clone();

    bool contains(int strain);
		std::vector<double> get_infectivity_trajectory(int strain);
		std::vector<double> get_symptomaticity_trajectory();

		void set_symptomaticity_trajectory(std::vector<double> symt);
		void set_infectivity_trajectory(int strain, std::vector<double> vlt);
    void set_infectivities(std::map<int, std::vector<double> > inf);
	
    std::map<int, double> *getCurrentLoads(int day);

		int get_duration() { return duration; }

		struct point {
			double infectivity;
			double symptomaticity;
			point( double infectivity_value, double symptomaticity_value) {
				infectivity = infectivity_value;
				symptomaticity = symptomaticity_value;
			};
		};
		
		point get_data_point(int t);

		class iterator {
			private:
				Trajectory * trajectory;
				int current;
				bool next_exists;
			public:
				iterator(Trajectory * trj) {
					trajectory = trj;
					next_exists = false;
					current = -1;
				}
				bool has_next() {
					current++;
					next_exists = ( current < trajectory->duration );
					return next_exists;
				}
				int get_current() {
					return current;
				}
				Trajectory::point next() {
					return trajectory->get_data_point(current);
				}
		};

		void calculate_aggregate_infectivity();
		std::map<int, double> *getInoculum(int day);

		std::string to_string();
		void print();		

	private:
		int duration;

		std::map< int, std::vector<double> > infectivity;
		std::vector<double> symptomaticity;		

		std::vector<double> aggregate_infectivity;
};

#endif


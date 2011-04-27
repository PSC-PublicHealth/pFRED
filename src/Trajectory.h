#ifndef _FRED_TRAJECTORY_H
#define _FRED_TRAJECTORY_H


#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <string>

typedef std::vector<double> trajectory_t;

class Trajectory {
  public:
    Trajectory();
    Trajectory(std::map< int, trajectory_t > infectivity_copy, trajectory_t symptomaticity_copy);
    Trajectory * clone();

    bool contains(int strain);
    trajectory_t get_infectivity_trajectory(int strain);
    trajectory_t get_symptomaticity_trajectory();
    void getAllStrains(std::vector<int> &);

    void set_symptomaticity_trajectory(trajectory_t symt);
    void set_infectivity_trajectory(int strain, trajectory_t vlt);
    void set_infectivities(std::map<int, trajectory_t > inf);

    std::map<int, double> *getCurrentLoads(int day);

    int get_duration() {
      return duration;
      }

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
    void modifySympPeriod(int startDate, int daysLeft);
    void modifyAsympPeriod(int startDate, int daysLeft, int sympDate);
    void modifyDevelopsSymp(int sympDate, int sympPeriod);

    std::string to_string();
    void print();

  private:
    int duration;

    std::map< int, trajectory_t > infectivity;
    trajectory_t symptomaticity;

    trajectory_t aggregate_infectivity;
  };

#endif


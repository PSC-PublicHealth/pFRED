/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Patch.h
//

#ifndef _FRED_PATCH_H
#define _FRED_PATCH_H

#include <vector>
#include <string.h>

#include "Place.h"
class Person;
class Grid;

class Patch {
  public:
    Patch() {}
    void setup(Grid * patch_mgr, int i, int j, double xmin, double xmax, double ymin, double ymax);
    void record_favorite_places();
    void print();
    void print_coord();
    double get_min_y() {
      return min_y;
      }
    double get_min_x() {
      return min_x;
      }
    double get_max_y() {
      return max_y;
      }
    double get_max_x() {
      return max_x;
      }
    double get_center_y() {
      return center_y;
      }
    double get_center_x() {
      return center_x;
      }
    void make_neighborhood();
    void add_household(Place *p);
    int get_houses() {
      return houses;
      }
    Place * get_neighborhood() {
      return neighborhood;
      }
    void enroll(Person *per) {
      neighborhood->enroll(per);
      }
    Place *select_random_household();
    Place *select_random_school(int age);
    Place *select_random_workplace();
    Person *select_random_person();
    Person *select_random_person_from_neighbors();
    Place *select_neighborhood();
    double distance_to_patch(Patch *p2);
    void quality_control();

  protected:
    Place * select_neighboring_patch();

    // set during setup():
    int row;
    int col;
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    double center_x;
    double center_y;
    Patch ** neighbors;
    Place * neighborhood;
    Grid * patch_manager;
    vector <Place *> household;
    int houses;

    // set during reset():
    vector <Place *> school[20];
    vector <Place *> workplace;
    vector <Person *> person;
  };

#endif // _FRED_PATCH_H

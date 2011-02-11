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
class Patches;

class Patch {
public:
  Patch() {}
  void setup(Patches * patch_mgr, int i, int j, double xmin, double xmax, double ymin, double ymax);
  void reset(int run);
  void print();
  void print_coord();
  double get_min_y() { return min_y;}
  double get_min_x() { return min_x;}
  double get_max_y() { return max_y;}
  double get_max_x() { return max_x;}
  void add_household(Place *p);
  int get_houses() { return houses;}
  Place * get_place() { return place; }
  Place * select_neighbor();
  void add_person_to_neighbors(Person *per);
  void add_person(Person *per) { place->add_person(per); }
  void set_social_networks();

protected:
  int row;
  int col;
  double min_x;
  double max_x;
  double min_y;
  double max_y;
  double center_x;
  double center_y;
  Patch ** neighbors;
  vector <Place *> household;
  vector <Place *> school[20];
  vector <Place *> workplace;
  vector <Person *> person;
  int houses;
  Place * place;
  Patches * patch_manager;
};

#endif // _FRED_PATCH_H

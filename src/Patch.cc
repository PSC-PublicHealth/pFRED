/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Patch.cc
//

#include <stdio.h>
#include <string>
using namespace std;
#include "Global.h"
#include "Patch.h"
#include "Patches.h"
#include "Place.h"
#include "Neighborhood.h"
#include "Locations.h"
#include "Random.h"
#include "Household.h"
#include "School.h"

class Person;

void Patch::setup(Patches * patch_mgr, int i, int j, double xmin, double xmax, double ymin, double ymax) {
  double lat, lon;
  patch_manager = patch_mgr;
  min_x = xmin;
  max_x = xmax;
  min_y = ymin;
  max_y = ymax;
  row = i;
  col = j;
  center_y = (min_y+max_y)/2.0;
  center_x = (min_x+max_x)/2.0;
  neighbors = patch_mgr->get_neighbors(i,j);
  houses = 0;
  household.clear();
  char str[80];
  sprintf(str, "Patch_%04d_%04d",row,col);
  int id = 900000000 + 10000*row + col;
  patch_mgr->translate_to_lat_lon(center_x,center_y,&lat,&lon);
  place = new (nothrow) Neighborhood(id, str, lon, lat, 0, &Pop);
  Loc.add_location(place);
  reset(0);
}

void Patch::print() {
  printf("Patch %d %d: %f, %f, %f, %f\n",row,col,min_x,max_x,min_y,max_y);
  for (int i = 0; i < 9; i++) {
    if (neighbors[i] == NULL) { printf("NULL ");}
    else {neighbors[i]->print_coord();}
    printf("\n");
  }
}


void Patch::print_coord() {
  printf("(%d, %d)",row,col);
}


void Patch::reset(int run) {
}

void Patch::add_household(Place *p) {
  houses++;
  household.push_back(p);
  if (Verbose > 1) {
    double lat, lon, x, y;
    lat = p->get_latitude();
    lon = p->get_longitude();
    patch_manager->translate_to_cartesian(lat,lon,&x,&y);
    printf("HHH %f %f\n", x,y);
  }
}

Place * Patch::select_neighbor() {
  // return place associated with current patch with 10% prob.
  if (RANDOM() < 0.1) return place;

  // otherwise select a random neighbor:

  int n = IRAND(0,8);
  if (neighbors[n] == NULL) 
    // return current patch if neighbor is off the grid
    return place;
  else
    // return place associated with selected neighbor
    return neighbors[n]->get_place();;
}

void Patch::add_person_to_neighbors(Person *per) {
  for (int i = 0; i < 9; i++) {
    if (neighbors[i] != NULL) {
      neighbors[i]->add_person(per);
    }
  }
}


void Patch::set_social_networks() {
  Household * house;
  Person * per;
  Place * p;
  School * s;
  for (int i = 0; i < houses; i++) {
    house = (Household *) household[i];
    household.push_back(house);
    int hsize = house->get_size();
    for (int j = 0; j < hsize; j++) {
      per = house->get_housemate(j);
      place->add_person(per);
      person.push_back(per);
      p = per->get_behavior()->get_workplace();
      if (p != NULL) workplace.push_back(p);
      s = (School *) per->get_behavior()->get_school();
      if (s != NULL) {
	for (int age = 0; age < 20; age++) {
	  if (s->children_in_grade(age) > 0)
	    school[age].push_back(s);
	}
      }
    }
  }
  printf("patch update: row = %d col = %d  pop = %d  houses = %d work = %d schools = ",
	 row,col,(int)person.size(),(int)household.size(),(int)workplace.size());
  for (int age = 0; age < 20; age++) {
    printf("%d ", (int)school[age].size());
  }
  printf("\n");
}


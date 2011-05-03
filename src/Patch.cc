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
#include "Grid.h"
#include "Place.h"
#include "Neighborhood.h"
#include "Place_List.h"
#include "Random.h"
#include "Household.h"
#include "School.h"

class Person;

void Patch::setup(Grid * patch_mgr, int i, int j, double xmin, double xmax, double ymin, double ymax) {
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
  make_neighborhood();
}

void Patch::make_neighborhood() {
  char str[80];
  double lat, lon;
  sprintf(str, "N-%04d-%04d",row,col);
  int id = Places.get_number_of_places();
  patch_manager->translate_to_lat_lon(center_x,center_y,&lat,&lon);
  neighborhood = new (nothrow) Neighborhood(id, str, lon, lat, 0, &Pop);
  Places.add_place(neighborhood);
}


void Patch::add_household(Place *p) {
  houses++;
  household.push_back(p);
  if (Verbose > 1) {
    double lat, lon, x, y;
    lat = p->get_latitude();
    lon = p->get_longitude();
    patch_manager->translate_to_cartesian(lat,lon,&x,&y);
    // p->print(0);
    // printf("HHH %f %f %f %f house_id: %d row = %d  col = %d  houses = %d\n", lon,lat, x,y, p->get_id(), row, col, houses);
  }
}

void Patch::record_favorite_places() {
  Household * house;
  Person * per;
  Place * p;
  School * s;

  // create lists of persons, workplaces, schools (by age)
  person.clear();
  workplace.clear();
  for (int age = 0; age < ADULT_AGE; age++) school[age].clear();

  for (int i = 0; i < houses; i++) {
    house = (Household *) household[i];
    int hsize = house->get_size();
    for (int j = 0; j < hsize; j++) {
      per = house->get_housemate(j);
      neighborhood->enroll(per);
      person.push_back(per);
      p = per->get_activities()->get_workplace();
      if (p != NULL) workplace.push_back(p);
      s = (School *) per->get_activities()->get_school();
      if (s != NULL) {
	for (int age = 0; age < ADULT_AGE; age++) {
	  if (s->children_in_grade(age) > 0)
	    school[age].push_back(s);
	}
      }
    }
  }
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

Place * Patch::select_neighborhood() {
  Patch * patch;
  double r = RANDOM();
  double grav_prob = 0.0;
  double self_prob = 0.5;

  if (r < grav_prob) {
    // use gravity model
    patch = patch_manager->select_patch_by_gravity_model(row,col);
  }
  else {
    r = (r - grav_prob)/(1.0 - grav_prob);
    if (r < self_prob) {
      // select local patch
      patch = this;
    }
    else {
      r = (r - self_prob)/(1.0 - self_prob);
      int n = (int) (8.0*r);
      patch = neighbors[n];
      if (patch == NULL) patch = this; // fall back to local patch
    }
  }
  // printf("DIST: %f\n", distance_to_patch(patch));
  return patch->get_neighborhood();
}


Person *Patch::select_random_person() {
  if ((int)person.size() == 0) return NULL;
  int i = IRAND(0, ((int) person.size())-1);
  return person[i];
}


Place *Patch::select_random_household() {
  if ((int)household.size() == 0) return NULL;
  int i = IRAND(0, ((int) household.size())-1);
  return household[i];
}


Place *Patch::select_random_workplace() {
  if ((int)workplace.size() == 0) return NULL;
  int i = IRAND(0, ((int) workplace.size())-1);
  return workplace[i];
}


Place *Patch::select_random_school(int age) {
  if ((int)school[age].size() == 0) return NULL;
  int i = IRAND(0, ((int) school[age].size())-1);
  return school[age][i];
}


Person *Patch::select_random_person_from_neighbors() {
  Patch * pat = NULL;
  Person * per = NULL;
  int trial = 0;
  while (per == NULL && trial < 20) {

    // select current patch with 10% prob.
    if (RANDOM() < 0.1) pat = this;

    // otherwise select a random neighbor:
    int n = IRAND(0,8);
    if (neighbors[n] == NULL) 
      // current patch if neighbor is off the grid
      pat = this;
    else
      pat = neighbors[n];

    // choose a random person from selected patch
    per = pat->select_random_person();
    trial++;
  }
  return per;
}

double Patch::distance_to_patch(Patch *p2) {
  double x1 = center_x;
  double y1 = center_y;
  double x2 = p2->get_center_x();
  double y2 = p2->get_center_y();
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

void Patch::quality_control() {
  return;
  fprintf(Statusfp,
	  "PATCH row = %d col = %d  pop = %d  houses = %d work = %d schools = ",
	  row,col,(int)person.size(),(int)household.size(),(int)workplace.size());
  for (int age = 0; age < 20; age++) {
    fprintf(Statusfp, "%d ", (int)school[age].size());
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}

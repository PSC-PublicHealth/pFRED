/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Cell.cc
//


/*
#include <stdio.h>
#include <string>
using namespace std;
*/
#include "Global.h"
#include "Grid.h"
#include "Cell.h"
#include "Place.h"
#include "Neighborhood.h"
#include "Place_List.h"
#include "Random.h"
#include "Household.h"
#include "School.h"
class Person;

void Cell::setup(Grid * grd, int i, int j, double xmin, double xmax, double ymin, double ymax) {
  grid = grd;
  min_x = xmin;
  max_x = xmax;
  min_y = ymin;
  max_y = ymax;
  row = i;
  col = j;
  center_y = (min_y+max_y)/2.0;
  center_x = (min_x+max_x)/2.0;
  neighbor_cells = (Cell **) grid->get_neighbors(i,j);
  houses = 0;
  occupied_houses = 0;
  household.clear();
  make_neighborhood();
}

void Cell::make_neighborhood() {
  char str[80];
  double lat, lon;
  sprintf(str, "N-%04d-%04d",row,col);
  int id = Global::Places.get_number_of_places();
  grid->translate_to_lat_lon(center_x,center_y,&lat,&lon);
  neighborhood = new (nothrow) Neighborhood(id, str, lon, lat, 0, &Global::Pop);
  Global::Places.add_place(neighborhood);
}

void Cell::add_household(Place *p) {
  houses++;
  household.push_back(p);
  if (Global::Verbose > 1) {
    double lat, lon, x, y;
    lat = p->get_latitude();
    lon = p->get_longitude();
    grid->translate_to_cartesian(lat,lon,&x,&y);
    // p->print(0);
    // printf("HHH %f %f %f %f house_id: %d row = %d  col = %d  houses = %d\n", lon,lat, x,y, p->get_id(), row, col, houses);
  }
}

void Cell::record_favorite_places() {
  Household * house;
  Person * per;
  Place * p;
  School * s;
  // FILE *fp;

  // create lists of persons, workplaces, schools (by age)
  person.clear();
  workplace.clear();
  for (int age = 0; age < Global::ADULT_AGE; age++) school[age].clear();

  // char filename[256];
  // sprintf(filename, "PATCHES/Cell-%d-%d-households", row, col);
  // fp = fopen(filename, "w");
  for (int i = 0; i < houses; i++) {
    house = (Household *) household[i];
    house->record_profile();
    int hsize = house->get_size();
    // fprintf(fp, "%d ", hsize);
    for (int j = 0; j < hsize; j++) {
      per = house->get_housemate(j);
      person.push_back(per);
      p = per->get_activities()->get_workplace();
      if (p != NULL) workplace.push_back(p);
      s = (School *) per->get_activities()->get_school();
      if (s != NULL) {
	for (int age = 0; age < Global::ADULT_AGE; age++) {
	  if (s->children_in_grade(age) > 0)
	    school[age].push_back(s);
	}
      }
    }
    // fprintf(fp, "\n");
  }
  // fclose(fp);
  // set target values
  target_popsize = (int) person.size();
  target_households = houses;
}


Place * Cell::select_neighborhood() {
  Cell * grid_cell;
  double r = RANDOM();
  double self_prob = 0.5;
  if (r < self_prob) {
    // select local grid_cell
    grid_cell = this;
  }
  else {
    r = (r - self_prob)/(1.0 - self_prob);
    int n = (int) (8.0*r);
    grid_cell = neighbor_cells[n];
    if (grid_cell == NULL) grid_cell = this; // fall back to local grid_cell
  }
  return grid_cell->get_neighborhood();
}


Person *Cell::select_random_person() {
  if ((int)person.size() == 0) return NULL;
  int i = IRAND(0, ((int) person.size())-1);
  return person[i];
}


Place *Cell::select_random_household() {
  if ((int)household.size() == 0) return NULL;
  int i = IRAND(0, ((int) household.size())-1);
  return household[i];
}


Place *Cell::select_random_workplace() {
  if ((int)workplace.size() == 0) return NULL;
  int i = IRAND(0, ((int) workplace.size())-1);
  return workplace[i];
}


Place *Cell::select_random_school(int age) {
  if ((int)school[age].size() == 0) return NULL;
  int i = IRAND(0, ((int) school[age].size())-1);
  return school[age][i];
}


Person *Cell::select_random_person_from_neighbors() {
  Cell * pat = NULL;
  Person * per = NULL;
  int trial = 0;
  while (per == NULL && trial < 20) {

    // select current grid_cell with 10% prob.
    if (RANDOM() < 0.1) pat = this;

    // otherwise select a random neighbor:
    int n = IRAND(0,8);
    if (neighbor_cells[n] == NULL)
      // current grid_cell if neighbor is off the array
      pat = this;
    else
      pat = neighbor_cells[n];

    // choose a random person from selected grid_cell
    per = pat->select_random_person();
    trial++;
  }
  return per;
}

void Cell::quality_control() {
  return;
  fprintf(Global::Statusfp,
	  "PATCH row = %d col = %d  pop = %d  houses = %d work = %d schools = ",
	  row,col,(int)person.size(),(int)household.size(),(int)workplace.size());
  for (int age = 0; age < 20; age++) {
    fprintf(Global::Statusfp, "%d ", (int)school[age].size());
  }
  fprintf(Global::Statusfp, "\n");
  fflush(Global::Statusfp);
}


void Cell::print() {
  printf("Cell %d %d: %f, %f, %f, %f\n",row,col,min_x,max_x,min_y,max_y);
  for (int i = 0; i < 9; i++) {
    if (neighbor_cells[i] == NULL) { printf("NULL ");}
    else {neighbor_cells[i]->print_coord();}
    printf("\n");
  }
}

void Cell::print_coord() {
  printf("(%d, %d)",row,col);
}

double Cell::distance_to_grid_cell(Cell *p2) {
  double x1 = center_x;
  double y1 = center_y;
  double x2 = p2->get_center_x();
  double y2 = p2->get_center_y();
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}






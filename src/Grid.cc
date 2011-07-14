/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Grid.cc
//

#include <utility>
#include <list>
#include <string>
using namespace std;

#include "Global.h"
#include "Place_List.h"
#include "Place.h"
#include "Grid.h"
#include "Params.h"
#include "Random.h"
#include "Utils.h"
#include "Household.h"
#include "Population.h"
#include "Date.h"

// global singleton object
Grid Environment;

void Grid::setup(double minlat, double maxlat, double minlon, double maxlon) {
  get_parameters();
  min_lat  = minlat;
  max_lat  = maxlat;
  min_lon  = minlon;
  max_lon  = maxlon;
  printf("min_lat = %f\n", min_lat);
  printf("max_lat = %f\n", max_lat);
  printf("min_lon = %f\n", min_lon);
  printf("max_lon = %f\n", max_lon);
  fflush(stdout);

  min_x = 0.0;
  max_x = (max_lon-min_lon)*km_per_deg_longitude;
  min_y = 0.0;
  max_y = (max_lat-min_lat)*km_per_deg_latitude;
  rows = 1 + (int) (max_y/patch_size);
  cols = 1 + (int) (max_x/patch_size);
  if (Global::Verbose) {
    printf("rows = %d  cols = %d\n",rows,cols);
    printf("max_x = %f  max_y = %f\n",max_x,max_y);
    fflush(stdout);
  }
  patch = new Patch*[rows];
  for (int i = 0; i < rows; i++) {
    patch[i] = new Patch[cols];
  }
  
  patch_pop = new int*[rows];
  for (int i = 0; i < rows; i++) {
    patch_pop[i] = new int[cols];
  }
  
  patch_prob = new double*[rows];
  for (int i = 0; i < rows; i++) {
    patch_prob[i] = new double[cols];
  }
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      patch[i][j].setup(this,i,j,j*patch_size,(j+1)*patch_size,
			(rows-i-1)*patch_size,(rows-i)*patch_size);
    }
  }

  if (Global::Verbose > 1) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	printf("print patch[%d][%d]:\n",i,j);
	patch[i][j].print();
      }
    }
  }
  vacant_houses.clear();
}

void Grid::get_parameters() {
  get_param((char *) "km_per_deg_longitude", &km_per_deg_longitude);
  get_param((char *) "km_per_deg_latitude", &km_per_deg_latitude);
  get_param((char *) "patch_size", &patch_size);
}

void Grid::make_neighborhoods() {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].make_neighborhood();
    }
  }
}

void Grid::record_favorite_places() {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].record_favorite_places();
      patch_pop[row][col] = patch[row][col].get_neighborhood()->get_size();
      target_popsize += patch_pop[row][col];
      target_households += patch[row][col].get_houses();
    }
  }
}

Patch * Grid::get_patch(int row, int col) {
  if ( row >= 0 && col >= 0 && row < rows && col < cols)
    return &patch[row][col];
  else
    return NULL;
}


Patch * Grid::select_random_patch() {
  int row = IRAND(0, rows-1);
  int col = IRAND(0, cols-1);
  return &patch[row][col];
}


Patch * Grid::get_patch_from_cartesian(double x, double y) {
  int row, col;
  row = rows-1 - (int) (y/patch_size);
  col = (int) (x/patch_size);
  // printf("x = %f y = %f, row = %d col = %d\n",x,y,row,col);
  return get_patch(row, col);
}


Patch * Grid::get_patch_from_lat_lon(double lat, double lon) {
  double x, y;
  translate_to_cartesian(lat,lon,&x,&y);
  return get_patch_from_cartesian(x,y);
}


void Grid::translate_to_cartesian(double lat, double lon, double *x, double *y) {
  *x = (lon - min_lon) * km_per_deg_longitude;
  *y = (lat - min_lat) * km_per_deg_latitude;
}


void Grid::translate_to_lat_lon(double x, double y, double *lat, double *lon) {
  *lon = min_lon + x * km_per_deg_longitude;
  *lat = min_lat + y * km_per_deg_latitude;
}


Patch ** Grid::get_neighbors(int row, int col) {
  Patch ** neighbors = new Patch*[9];
  int n = 0;
  for (int i = row-1; i <= row+1; i++) {
    for (int j = col-1; j <= col+1; j++) {
      neighbors[n++] = get_patch(i,j);
    }
  }
  return neighbors;
}


Patch * Grid::select_patch_by_gravity_model(int row, int col) {
  // compute patch probabilities for gravity model
  Patch * p1 = &patch[row][col];
  double x1 = p1->get_center_x();
 double y1 = p1->get_center_y();

  double total = 0.0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      Patch * p2 = &patch[i][j];
      double x2 = p2->get_center_x();
      double y2 = p2->get_center_y();
      double pop2 = (double) p2->get_neighborhood()->get_size();
      double dist = ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
      if (dist == 0.0) {
	dist = 0.5*patch_size*0.5*patch_size;
      }
      patch_prob[i][j] = pop2 / dist;
      total += patch_prob[i][j];
    }
  }

  // select a patch at random using the computed probabilities
  double r = RANDOM()*total;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (r < patch_prob[i][j]) {
	return &patch[i][j];
      }
      else {
	r -= patch_prob[i][j];
      }
    }
  }
  Utils::fred_abort("Help! patch gravity model failed.\n");

  //Will never get here, but will stop compiler warning
  return NULL;
}

void Grid::test_gravity_model() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      for (int n = 0; n < 1000; n++) {
	Patch * p1 = &patch[i][j];
	Patch * p2 = select_patch_by_gravity_model(i,j);
	double x1 = p1->get_center_x();
	double y1 = p1->get_center_y();
	double x2 = p2->get_center_x();
	double y2 = p2->get_center_y();
	double dist = ((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
	printf("DIST: %f\n", dist);
      }
    }
  }
  exit(0);
}

void Grid::quality_control() {
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "patches quality control check\n");
    fflush(Global::Statusfp);
  }
  
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      patch[row][col].quality_control();
    }
  }

  if (Global::Verbose) {
    fprintf(Global::Statusfp, "patches quality control finished\n");
    fflush(Global::Statusfp);
  }
}


void Grid::add_vacant_house(Place * house) {
  vacant_houses.push_back(house);
}

Place * Grid::get_vacant_house() {
  Place * house = NULL;
  int count = vacant_houses.size();
  if (count > 0) {
    // pick a random vacant house
    int i = IRAND(0, count-1);
    house = vacant_houses[i];
    vacant_houses[i] = vacant_houses.back();
    vacant_houses.pop_back();
  }
  return house;
}

void Grid::population_migration() {
  select_emigrants();
  select_immigrants();
}


void Grid::select_emigrants() {
  Person * resident[100];
  if (Global::Verbose) {
    printf("EMIG oldpopsize = %d\n", Global::Pop.get_pop_size());
  }
  double epct;
  // monthly prob of emigrating
  epct = 0.02/12.0;
  int houses_to_emigrate;
  houses_to_emigrate = epct * target_households;
  printf("houses_to_emigrate = %d\n", houses_to_emigrate); fflush(stdout);
  int houses_vacated = 0;
  int people_removed = 0;
  for (int h = 0; h < houses_to_emigrate; h++) {

    Person * per = Global::Pop.select_random_person();
    if (RANDOM() < 0.5) {
      // pick a random person between 50 and 60
      int age = per->get_age();
      while (age < 50 || age > 60) {
	per = Global::Pop.select_random_person();
	age = per->get_age();
      }
    }
    Household * house_to_vacate = (Household *) per->get_household();

    // vacate the house
    int size = house_to_vacate->get_size();
    printf("house_to_vacate = %d  size = %d\n", house_to_vacate->get_id(), size); fflush(stdout);

    // get a list of all housemates:
    for (int i = 0; i < size; i++) { resident[i] = house_to_vacate->get_housemate(i); }

    for (int i = 0; i < size; i++) {
      Person * emigrant = resident[i];
      // unenroll for all favorite places, including the house
      printf("person_to_emigrate = %d  age = %d\n", emigrant->get_id(), emigrant->get_age()); fflush(stdout);
      printf("deleting from population\n"); fflush(stdout);
      // remove from population after withdrawing from activities
      Global::Pop.delete_person(emigrant);
      printf("deleted from population\n"); fflush(stdout);
      people_removed++;
    }
    houses_vacated++;
  }
  if (Global::Verbose) {
    printf("newpopsize = %d  people_removed = %d houses_vacated = %d\n",
	   Global::Pop.get_pop_size(), people_removed, houses_vacated);
    fflush(stdout);
  }
}

void Grid::select_immigrants() {
  int current_year = Global::Sim_Date->get_year();
  int current_popsize = Global::Pop.get_pop_size();
  printf("IMM curr = %d target = %d ", current_popsize, target_popsize);
  printf("vacant houses = %d  current_year = %d\n", get_vacant_houses(), current_year);
  int n = 0;
  int houses_filled = 0;
  while (current_popsize < target_popsize && get_vacant_houses() > 0) {
    // pick a vacant house
    Place * vacant = get_vacant_house();
    printf("vacant house = %d\n", vacant->get_id());
    // find its patch
    Patch * p = vacant->get_patch();
    printf("Patch row = %d col = %d\n", p->get_row(), p->get_col());
    // pick a random household from the patch
    Household * clone_house = (Household *) p->select_random_household();
    int size = clone_house->get_orig_size();
    printf("IMM: clone house = %d size = %d\n", clone_house->get_id(), size);
    houses_filled++;
    // clone the original residents of this house
    for (int i = 0; i < size; i++) {
      // clone the ith original housemate
      int idx = clone_house->get_orig_id(i);
      char pstring[256];
      strcpy(pstring, Global::Pop.get_pstring(idx));

      int next_id = Global::Pop.get_next_id();
      Person * clone = new Person;
      int age, married, occ;
      char label[32], house[32], school[32], work[32];
      char sex;
      sscanf(pstring, "%s %d %c %d %d %s %s %s",
	     label, &age, &sex, &married, &occ, house, school, work);

      // make younger to reflect age based on next decennial
      int year_diff = 2010-current_year;
      if (age >= year_diff) age = age - year_diff;

      // redirect to the vacant house
      strcpy(house, vacant->get_label());

      // setup to clone
      clone->setup(next_id, age, sex, married, occ, house, school, work, &Global::Pop, Global::Sim_Date, true);

      // add to the popualtion
      Global::Pop.add_person(clone);

      // enroll on favorite places
      clone->reset(Global::Sim_Date);

      clone->print(stdout,0);
      current_popsize++;
      n++;
    }
  }
  if (houses_filled > 0) {
    printf("IMM: %d house filled, %d people added, new popsize = %d = %d\n",
	   houses_filled, n, current_popsize, Global::Pop.get_pop_size());
    fflush(stdout);
  }
}

void Grid::print_household_distribution(char * dir, char * date_string, int run) {
  FILE *fp;
  int targ, count;
  double pct;
  char filename[256];
  sprintf(filename, "%s/household_dist_%s.%02d", dir, date_string, run);
  printf("print_household_dist entered, filename = %s\n", filename); fflush(stdout);

  fp = fopen(filename, "w");
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      count = patch[i][j].get_occupied_houses();
      targ = patch[i][j].get_target_households();
      if (targ > 0) pct = (100.0*count)/targ;
      else pct = 0.0;
      fprintf(fp, "%d %d %d %d %f\n", i, j, targ, count, pct);
    }
  }
  fclose(fp);
}


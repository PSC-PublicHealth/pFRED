/*
  This file is part of the FRED system.

  Copyright (c) 2010-2012, University of Pittsburgh, John Grefenstette,
  Shawn Brown, Roni Rosenfield, Alona Fyshe, David Galloway, Nathan
  Stone, Jay DePasse, Anuroop Sriram, and Donald Burke.

  Licensed under the BSD 3-Clause license.  See the file "LICENSE" for
  more information.
*/

//
//
// File: Large_Cell.cc
//
#include <algorithm>

#include "Global.h"
#include "Large_Cell.h"
#include "Large_Grid.h"
#include "Geo_Utils.h"
#include "Random.h"
#include "Utils.h"

int Large_Cell::next_cell_id = 0;

void Large_Cell::setup(Large_Grid * grd, int i, int j) {
  grid = grd;
  row = i;
  col = j;
  double grid_cell_size = grid->get_grid_cell_size();
  double grid_min_x = grid->get_min_x();
  double grid_min_y = grid->get_min_y();
  min_x = grid_min_x + (col)*grid_cell_size;
  min_y = grid_min_y + (row)*grid_cell_size;
  max_x = grid_min_x + (col+1)*grid_cell_size;
  max_y = grid_min_y + (row+1)*grid_cell_size;
  center_y = (min_y+max_y)/2.0;
  center_x = (min_x+max_x)/2.0;
  popsize = 0;
  max_popsize = 0;
  pop_density = 0;
  person.clear();
  workplaces.clear();
  id = next_cell_id++;
}

//void Large_Cell::print() {
//  printf("Large_Cell %d %d: %f, %f, %f, %f\n",row,col,min_x,max_x,min_y,max_y);
//  for (int i = 0; i < 9; i++) {
//    if (neighbors[i] == NULL) { printf("NULL ");}
//    else {neighbors[i]->print_coord();}
//    printf("\n");
//  }
//}

void Large_Cell::quality_control() {
  return;
}

double Large_Cell::distance_to_grid_cell(Large_Cell *p2) {
  double x1 = center_x;
  double y1 = center_y;
  double x2 = p2->get_center_x();
  double y2 = p2->get_center_y();
  return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}


Person *Large_Cell::select_random_person() {
  if ((int)person.size() == 0) return NULL;
  int i = IRAND(0, ((int) person.size())-1);

  //if (Global::Verbose > 2) {
    //fprintf(Global::Statusfp, "ABC%d,%d\n", i, person[i]->get_id());
    //fflush(Global::Statusfp);
    //fprintf(stderr, "ABC%d,%d\n", i, person[i]->get_id());
    //fflush(stderr);
  //}
  return person[i];
}


void Large_Cell::set_max_popsize(int n) {
  max_popsize = n; 
  pop_density = (double) popsize/ (double) n;
  
  // the following reflects noise in the estimated population in the preprocessing routine
  if (pop_density > 0.8) pop_density = 1.0;

  /*
    if (pop_density < 1.0) { printf("WARNING: cell %d %d pop %d max_pop %d frac %f\n",
    col,row,popsize,max_popsize,pop_density);
    fflush(stdout);
  */
}

void Large_Cell::unenroll(Person *per) {
  // <-------------------------------------------------------------- Mutex
  fred::Scoped_Lock lock( mutex );
  /* Why look by id and not just find and remove the pointer using std algorithms?
  int id = per->get_id();
  vector<Person *>::iterator it;
  for(it = person.begin(); it != person.end(); it++) {
    if(id == (*it)->get_id()) break;
  }
  if(it != person.end()) {
    person.erase(it);
  }
  */
  if ( person.size() > 1 ) {
    std::vector< Person * >::iterator iter;
    iter = std::find( person.begin(), person.end(), per );
    if ( iter != person.end() ) {
      std::swap( (*iter), person.back() );
      person.erase( person.end() - 1 );
    }
    assert( std::find( person.begin(), person.end(), per ) == person.end() );
  }
  else {
    person.clear();
  }
}

Transaction * Large_Cell::collect_cell_stats( int day, int disease_id ) {
  Cell_Report * report = new Cell_Report( day, id, disease_id );
  report->collect( person );
  return report;
}

Place *Large_Cell::get_workplace_near_to_school(Place *school) {
  // printf("get_workplace_near_school entered\n"); print(); fflush(stdout);
  int size = school->get_size();
  double x = Geo_Utils::get_x(school->get_longitude());
  double y = Geo_Utils::get_y(school->get_latitude());

  //from: http://www.statemaster.com/graph/edu_ele_sec_pup_rat-elementary-secondary-pupil-teacher-ratio
  int staff = 1 + (int) (0.5 + size / 15.5);
  int min_staff = (int) (0.75 * staff);
  if (min_staff < 1) min_staff = 1;
  int max_staff = (int) (0.5 + 1.25 * staff);
  FRED_VERBOSE( 0, " size %d staff %d %d %d \n", size, min_staff, staff, max_staff);

  // find nearest workplace that has right number of employees
  double min_dist = 1e99;
  Place * nearby_workplace = grid->get_nearby_workplace(row,col,x,y,min_staff,max_staff,&min_dist);
  if (nearby_workplace == NULL) return NULL;
  assert(nearby_workplace != NULL);
  double x2 = Geo_Utils::get_x(nearby_workplace->get_longitude());
  double y2 = Geo_Utils::get_y(nearby_workplace->get_latitude());
  FRED_VERBOSE(0, "nearby workplace %s %f %f wsize %d work_dist %f\n", nearby_workplace->get_label(), x2, y2, nearby_workplace->get_size(), min_dist);

  return nearby_workplace;
}


Place * Large_Cell::get_closest_workplace(double x, double y, int min_size, int max_size, double * min_dist) {
  // printf("get_closest_workplace entered for cell %d %d\n", row, col); fflush(stdout);
  Place * closest_workplace = NULL;
  int number_workplaces = workplaces.size();
  for (int j = 0; j < number_workplaces; j++) {
    Place *workplace = workplaces[j];
    int size = workplace->get_size();
    if (min_size <= size && size <= max_size) {
      double x2 = Geo_Utils::get_x(workplace->get_longitude());
      double y2 = Geo_Utils::get_y(workplace->get_latitude());
      double dist = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
      if (dist < 20.0 && dist < *min_dist) {
	*min_dist = dist;
	closest_workplace = workplace;
	// printf("closer = %s size = %d min_dist = %f\n", closest_workplace->get_label(), size, *min_dist);
	// fflush(stdout);
      }
    }
  }
  return closest_workplace;
}

void Large_Cell::add_workplace(Place *workplace) {
  workplaces.push_back(workplace);
  // double x = Geo_Utils::get_x(workplace->get_longitude());
  // double y = Geo_Utils::get_y(workplace->get_latitude());
  // printf("ADD WORK: to large cell (%d, %d) %s %f %f\n", row,col,workplace->get_label(),x,y); fflush(stdout);
}

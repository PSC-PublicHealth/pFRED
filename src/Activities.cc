/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Activities.cc
//

#include "Activities.h"
#include "Behavior.h"
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#include "Date.h"
#include "Cell.h"
#include "Grid.h"
#include "Geo_Utils.h"
#include "Household.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Place_List.h"
#include "Utils.h"
#include "Travel.h"

#define PRESCHOOL_PROFILE 0
#define STUDENT_PROFILE 1
#define TEACHER_PROFILE 2
#define WORKER_PROFILE 3
#define WEEKEND_WORKER_PROFILE 4
#define UNEMPLOYED_PROFILE 5
#define RETIRED_PROFILE 6

bool Activities::is_initialized = false;
double Activities::age_yearly_mobility_rate[MAX_MOBILITY_AGE + 1];
bool Activities::is_weekday = false;
int Activities::day_of_week = 0;
double Activities::Community_distance = 20;
double Activities::Community_prob = 0.1;
double Activities::Home_neighborhood_prob = 0.5;
double Activities::Unauthorized_sick_leave_prob = 0.0;
int Activities::Sick_days_worked = 0;
int Activities::Sick_days_absent = 0;
int Activities::School_sick_days_attended = 0;
int Activities::School_sick_days_absent = 0;
double Activities::sick_day_prob = 0.0;

Activities::Activities (Person *person, Place *house, Place *school, Place *work) {
  //Create the static arrays one time
  if (!Activities::is_initialized) {
    read_init_files();
    Activities::is_initialized = true;
  }
  self = person;

  for (int i = 0; i < FAVORITE_PLACES; i++) {
    favorite_place[i] = NULL;
  }
  favorite_place[HOUSEHOLD_INDEX] = house;
  favorite_place[SCHOOL_INDEX] = school;
  favorite_place[WORKPLACE_INDEX] = work;
  assert(get_household() != NULL);

  // get the neighbhood from the household
  favorite_place[NEIGHBORHOOD_INDEX] =
    favorite_place[HOUSEHOLD_INDEX]->get_grid_cell()->get_neighborhood();
  if (get_neighborhood() == NULL) {
    printf("Help! NO NEIGHBORHOOD for person %d house %d \n",
           self->get_id(), get_household()->get_id());
  }
  assert(get_neighborhood() != NULL);
  assign_profile();

  // enroll in all the favorite places
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (favorite_place[i] != NULL) {
      favorite_place[i]->enroll(self);
    }
  }
  // need to set the daily schedule
  schedule_updated = -1;
  travel_status = false;
  traveling_outside = false;
  days_absent = -1;
}

void Activities::assign_profile() {
  int age = self->get_age();
  if (age < Global::SCHOOL_AGE && get_school() == NULL)
    profile = PRESCHOOL_PROFILE;    // child at home
  else if (age < Global::SCHOOL_AGE && get_school() != NULL)
    profile = STUDENT_PROFILE;      // child in preschool
  else if (age < Global::ADULT_AGE && get_school() != NULL)
    profile = STUDENT_PROFILE;      // student
  else if (get_school() != NULL)
    profile = TEACHER_PROFILE;      // teacher
  else if (Global::RETIREMENT_AGE <= age && RANDOM() < 0.5)
    profile = RETIRED_PROFILE;      // retired
  else
    profile = WORKER_PROFILE;     // worker

  // weekend worker
  if (profile == WORKER_PROFILE && RANDOM() < 0.2) {
    profile = WEEKEND_WORKER_PROFILE;   // 20% weekend worker
  }

  // unemployed
  if ((profile == WORKER_PROFILE ||
       profile == WEEKEND_WORKER_PROFILE) && RANDOM() < 0.1) {
    profile = UNEMPLOYED_PROFILE;   // 10% unemployed
  }
}

void Activities::update(int day) {

  // decide if this is a weekday:
  Activities::day_of_week = Date::get_current_day_of_week(day);
  Activities::is_weekday = (0 < Activities::day_of_week && Activities::day_of_week < 6);

  if (day>0) {
    printf("DAY %d ABSENTEEISM: work abs %d att %d %0.2f  school abs %d att %d %0.2f\n", day-1,
	   Activities::Sick_days_absent,
	   Activities::Sick_days_worked,
	   (double) (Activities::Sick_days_absent) /(double)(1+Activities::Sick_days_absent+Activities::Sick_days_worked),
	   Activities::School_sick_days_absent,
	   Activities::School_sick_days_attended,
	   (double) (Activities::School_sick_days_absent) /(double)(1+Activities::School_sick_days_absent+Activities::School_sick_days_attended));
  }

  // keep track of global activity counts
  Activities::Sick_days_worked = 0;
  Activities::Sick_days_absent = 0;
  Activities::School_sick_days_attended = 0;
  Activities::School_sick_days_absent = 0;
}


void Activities::update_infectious_activities(int day, int dis) {
  // skip scheduled activities if traveling abroad
  if (traveling_outside)
    return;

  // get list of places to visit today
  update_schedule(day);

  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (on_schedule[i]) {
      assert(favorite_place[i] != NULL);
      favorite_place[i]->add_infectious(dis, self);
    }
  }
}


void Activities::update_susceptible_activities(int day, int dis) {
  // skip scheduled activities if traveling abroad
  if (traveling_outside)
    return;

  // get list of places to visit today
  update_schedule(day);

  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (on_schedule[i]) {
      assert(favorite_place[i] != NULL);
      if (favorite_place[i]->is_infectious(dis)) {
	favorite_place[i]->add_susceptible(dis, self);
      }
    }
  }
}


void Activities::update_schedule(int day) {
  // update this schedule only once per day
  if (day <= schedule_updated)
    return;
  schedule_updated = day;

  for (int p = 0; p < FAVORITE_PLACES; p++) {
    on_schedule[p] = false;
  }

  // always visit the household
  on_schedule[HOUSEHOLD_INDEX] = true;

  // provisionally visit the neighborhood
  on_schedule[NEIGHBORHOOD_INDEX] = true;

  // weekday vs weekend provisional activity
  if (Activities::is_weekday) {
    if (favorite_place[SCHOOL_INDEX] != NULL)
      on_schedule[SCHOOL_INDEX] = true;
    if (favorite_place[CLASSROOM_INDEX] != NULL)
      on_schedule[CLASSROOM_INDEX] = true;
    if (favorite_place[WORKPLACE_INDEX] != NULL)
      on_schedule[WORKPLACE_INDEX] = true;
    if (favorite_place[OFFICE_INDEX] != NULL)
      on_schedule[OFFICE_INDEX] = true;
  }
  else {
    if (profile == WEEKEND_WORKER_PROFILE || profile == STUDENT_PROFILE) {
      if (favorite_place[WORKPLACE_INDEX] != NULL)
	on_schedule[WORKPLACE_INDEX] = true;
      if (favorite_place[OFFICE_INDEX] != NULL)
	on_schedule[OFFICE_INDEX] = true;
    }
  }

  // skip work at background absenteeism rate
  if (Global::Work_absenteeism > 0.0 && on_schedule[WORKPLACE_INDEX]) {
    if (RANDOM() < Global::Work_absenteeism)
      on_schedule[WORKPLACE_INDEX] = false;
      on_schedule[OFFICE_INDEX] = false;
  }

  // skip school at background school absenteeism rate
  if (Global::School_absenteeism > 0.0 && on_schedule[SCHOOL_INDEX]) {
    if (RANDOM() < Global::School_absenteeism)
      on_schedule[SCHOOL_INDEX] = false;
      on_schedule[CLASSROOM_INDEX] = false;
  }

  // decide whether to stay home if symptomatic
  if (self->is_symptomatic()) {
    bool taking_sick_day = false;
  
    if (self->is_adult()) {
      // sick adult
      bool stay_home = false;
      bool take_sick_leave = false;
      if (Global::Enable_Behaviors) {
	stay_home = self->get_behavior()->adult_is_staying_home(day);
	take_sick_leave = self->get_behavior()->adult_is_taking_sick_leave(day);
      }
      else {
	// default sick leave behavior
	stay_home = (RANDOM() < Activities::sick_day_prob);
      }

      bool authorized_sick_leave = false;
      if (on_schedule[WORKPLACE_INDEX]) {
	Workplace *work = (Workplace *) favorite_place[WORKPLACE_INDEX];
	authorized_sick_leave = work->is_sick_leave_available();
      }

      if (authorized_sick_leave) {
	// the following results in 70% of those taking sick leave taking 2 days off
	// at the start of the symptomatic period, and 30% take one day off.
	// this gives and average of 1.7 days off for those taking sick leave.
	if (take_sick_leave) {
	  if (days_absent == -1) {
	    if (RANDOM() < 0.7) days_absent = 2;
	    else days_absent = 1;
	  }
	  if (days_absent > 0)
	    days_absent--;
	  else
	    take_sick_leave = false;
	}
      }
      else {
	if (on_schedule[WORKPLACE_INDEX]) {
	  take_sick_leave = (RANDOM() < Activities::Unauthorized_sick_leave_prob);
	}
      }

      // decide if staying home for any reason
      taking_sick_day = stay_home || take_sick_leave;

      // record sick day decision for workdays
      if (on_schedule[WORKPLACE_INDEX]) {
	if (taking_sick_day) {
	  Activities::Sick_days_absent++;
	  /*
	  if (take_sick_leave) {
	    printf("Sick leave person %d days_absent = %d\n", self->get_id(), days_absent+1);
	  }
	  */
	}
	else {
	  Activities::Sick_days_worked++;
	}
      }
    }
    else {
      // sick child
      if (Global::Enable_Behaviors) {
	taking_sick_day = self->get_behavior()->child_is_staying_home(day);
      }
      else {
	// default baseline behavior
	taking_sick_day = (RANDOM() < Activities::sick_day_prob);
      }
      // record sick day decision for school days
      if (on_schedule[SCHOOL_INDEX]) {
	if (taking_sick_day)
	  Activities::School_sick_days_absent++;
	else
	  Activities::School_sick_days_attended++;
      }
    }

    if (taking_sick_day) {
      // withdraw to household
      on_schedule[WORKPLACE_INDEX] = false;
      on_schedule[OFFICE_INDEX] = false;
      on_schedule[SCHOOL_INDEX] = false;
      on_schedule[CLASSROOM_INDEX] = false;
      on_schedule[NEIGHBORHOOD_INDEX] = false;
    }
  }

  // decide which neighborhood to visit today
  if (on_schedule[NEIGHBORHOOD_INDEX]) {
    favorite_place[NEIGHBORHOOD_INDEX] =
      favorite_place[HOUSEHOLD_INDEX]->select_neighborhood(Activities::Community_prob,
							   Activities::Community_distance,
							   Activities::Home_neighborhood_prob);
  }

  if (Global::Verbose > 2) {
    fprintf(Global::Statusfp, "update_schedule on day %d\n", day);
    print_schedule(day);
    fflush(Global::Statusfp);
  }
}

void Activities::print_schedule(int day) {
  fprintf(Global::Statusfp, "day %d schedule for person %d  ", day, self->get_id());
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    fprintf(Global::Statusfp, on_schedule[p]? "+" : "-");
    if (favorite_place[p] == NULL) {
      fprintf(Global::Statusfp, "-1 ");
    } else {
      fprintf(Global::Statusfp, "%d ", favorite_place[p]->get_id());
    }
  }
  fprintf(Global::Statusfp, "\n");
  fflush(Global::Statusfp);
}

void Activities::print() {
  fprintf(Global::Statusfp, "Activities for person %d: ", self->get_id());
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL)
      fprintf(Global::Statusfp, "%d ", favorite_place[p]->get_id());
    else {
      fprintf(Global::Statusfp, "-1 ");
    }
  }
  fprintf(Global::Statusfp, "\n");
  fflush(Global::Statusfp);
}

void Activities::assign_school() {
  int age = self->get_age();
  if (age < Global::SCHOOL_AGE || Global::ADULT_AGE <= age) return;
  Cell *grid_cell = favorite_place[HOUSEHOLD_INDEX]->get_grid_cell();
  assert(grid_cell != NULL);
  Place *p = grid_cell->select_random_school(age);
  if (p != NULL) {
    favorite_place[SCHOOL_INDEX] = p;
    favorite_place[CLASSROOM_INDEX] = NULL;
    assign_classroom();
    return;
  }
  int trials = 0;
  while (trials < 100) {
    grid_cell = (Cell *) Global::Cells->select_random_grid_cell();
    p = grid_cell->select_random_school(age);
    if (p != NULL) {
      favorite_place[SCHOOL_INDEX] = p;
      favorite_place[CLASSROOM_INDEX] = NULL;
      assign_classroom();
      return;
    }
    trials++;
  }
  Utils::fred_abort("assign_school: can't locate school for person %d\n", self->get_id());
}

void Activities::assign_classroom() {
  if (favorite_place[SCHOOL_INDEX] != NULL &&
      favorite_place[CLASSROOM_INDEX] == NULL) {
    Place * place =
      ((School *) favorite_place[SCHOOL_INDEX])->assign_classroom(self);
    if (place != NULL) place->enroll(self);
    favorite_place[CLASSROOM_INDEX] = place;
  }
}

void Activities::assign_workplace() {
  int age = self->get_age();
  if (age < Global::ADULT_AGE) return;
  Cell *grid_cell = favorite_place[HOUSEHOLD_INDEX]->get_grid_cell();
  assert(grid_cell != NULL);
  Place *p = grid_cell->select_random_workplace();
  if (p != NULL) {
    favorite_place[WORKPLACE_INDEX] = p;
    favorite_place[OFFICE_INDEX] = NULL;
    assign_office();
    return;
  }
  int trials = 0;
  while (trials < 100) {
    grid_cell = (Cell *) Global::Cells->select_random_grid_cell();
    p = grid_cell->select_random_workplace();
    if (p != NULL) {
      favorite_place[WORKPLACE_INDEX] = p;
      favorite_place[OFFICE_INDEX] = NULL;
      assign_office();
      return;
    }
    trials++;
  }
  Utils::fred_abort("assign_workplace: can't locate workplace for person %d\n", self->get_id());
}

void Activities::assign_office() {
  if (favorite_place[WORKPLACE_INDEX] != NULL &&
      favorite_place[OFFICE_INDEX] == NULL) {
    Place * place =
      ((Workplace *) favorite_place[WORKPLACE_INDEX])->assign_office(self);
    if (place != NULL) place->enroll(self);
    favorite_place[OFFICE_INDEX] = place;
  }
}

int Activities::get_group_size(int index) {
  int size = 0;
  if (favorite_place[index] != NULL)
    size = favorite_place[index]->get_size();
  return size;
}

int Activities::get_degree() {
  int degree;
  int n;
  degree = 0;
  n = get_group_size(NEIGHBORHOOD_INDEX);
  if (n > 0) degree += (n-1);
  n = get_group_size(SCHOOL_INDEX);
  if (n > 0) degree += (n-1);
  n = get_group_size(WORKPLACE_INDEX);
  if (n > 0) degree += (n-1);
  return degree;
}

void Activities::update_profile() {
  int age = self->get_age();

  if (profile == PRESCHOOL_PROFILE && Global::SCHOOL_AGE <= age && age < Global::ADULT_AGE) {
    // start school
    profile = STUDENT_PROFILE;
    // select a school based on age and neighborhood
    assign_school();
    if (Global::Verbose>1) {
      fprintf(Global::Statusfp,
	      "changed behavior profile to STUDENT: id %d age %d sex %c\n",
	      self->get_id(), age, self->get_sex());
      print();
      fflush(Global::Statusfp);
    }
    return;
  }

  if (profile == STUDENT_PROFILE && age < Global::ADULT_AGE) {
    // select a school based on age and neighborhood
    School * s = (School *) favorite_place[SCHOOL_INDEX];
    Classroom * c = (Classroom *) favorite_place[CLASSROOM_INDEX];
    if (c == NULL || c->get_age_level() == age) {
      // no change
      if (Global::Verbose>1) {
	fprintf(Global::Statusfp,
		"KEPT CLASSROOM ASSIGNMENT: id %d age %d sex %c ",
		self->get_id(), age, self->get_sex());
	fprintf(Global::Statusfp, "%s %s | ",
		s->get_label(), c->get_label());
	print();
	fflush(Global::Statusfp);
      }
      return;
    } else if (s != NULL && s->classrooms_for_age(age) > 0) {
      // pick a new classrooms in current school
      favorite_place[CLASSROOM_INDEX] = NULL;
      assign_classroom();
      if (Global::Verbose>1) {
	fprintf(Global::Statusfp,
		"CHANGED CLASSROOM ASSIGNMENT: id %d age %d sex %c ",
		self->get_id(), age, self->get_sex());
	if (s != NULL && c != NULL) {
	  fprintf(Global::Statusfp, "from %s %s to %s %s | ",
		  s->get_label(), c->get_label(),
		  favorite_place[SCHOOL_INDEX]->get_label(),
		  favorite_place[CLASSROOM_INDEX]->get_label());
	} else {
	  fprintf(Global::Statusfp, "from NULL NULL to %s %s | ",
		  favorite_place[SCHOOL_INDEX]->get_label(),
		  favorite_place[CLASSROOM_INDEX]->get_label());
	}
	print();
	fflush(Global::Statusfp);
      }
    } else {
      // pick a new school and classroom
      assign_school();
      if (Global::Verbose>1) {
	fprintf(Global::Statusfp,
		"CHANGED SCHOOL ASSIGNMENT: id %d age %d sex %c ",
		self->get_id(), age, self->get_sex());
	if (s != NULL && c != NULL) {
	  fprintf(Global::Statusfp, "from %s %s to %s %s | ",
		  s->get_label(), c->get_label(),
		  favorite_place[SCHOOL_INDEX]->get_label(),
		  favorite_place[CLASSROOM_INDEX]->get_label());
	} else {
	  fprintf(Global::Statusfp, "from NULL NULL to %s %s | ",
		  favorite_place[SCHOOL_INDEX]->get_label(),
		  (favorite_place[CLASSROOM_INDEX] == NULL)?
		  "NULL" : favorite_place[CLASSROOM_INDEX]->get_label());
	}
	print();
	fflush(Global::Statusfp);
      }
    }
    return;
  }

  if (profile == STUDENT_PROFILE && Global::ADULT_AGE <= age) {
    // leave school
    favorite_place[SCHOOL_INDEX] = NULL;
    favorite_place[CLASSROOM_INDEX] = NULL;
    // get a job
    profile = WORKER_PROFILE;
    assign_workplace();
    if (Global::Verbose>1) {
      fprintf(Global::Statusfp,
	      "changed behavior profile to WORKER: id %d age %d sex %c\n",
	      self->get_id(), age, self->get_sex());
      print();
      fflush(Global::Statusfp);
    }
    return;
  }

  if (profile != RETIRED_PROFILE && Global::RETIREMENT_AGE <= age) {
    if (RANDOM() < 0.5) {
      // quit working
      favorite_place[WORKPLACE_INDEX] = NULL;
      favorite_place[OFFICE_INDEX] = NULL;
      profile = RETIRED_PROFILE;
      if (Global::Verbose>1) {
	fprintf(Global::Statusfp,
		"changed behavior profile to RETIRED: age %d age %d sex %c\n",
		self->get_id(), age, self->get_sex());
	print();
	fflush(Global::Statusfp);
      }
    }
    return;
  }
}

static int mobility_count[MAX_MOBILITY_AGE + 1];
static int mobility_moved[MAX_MOBILITY_AGE + 1];
static int mcount = 0;

void Activities::update_household_mobility() {
  if (!Global::Enable_Mobility) return;
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp, "update_household_mobility entered with mcount = %d\n", mcount);
    fflush(Global::Statusfp);
  }
  
  if (mcount == 0) {
    for (int i = 0; i <= MAX_MOBILITY_AGE; i++) {
      mobility_count[i] = mobility_moved[i] = 0;
    }
  }

  int age = self->get_age();
  mobility_count[age]++;
  mcount++;

  Household * household = (Household *) self->get_household();
  if (self->is_householder()) {
    if (RANDOM() < Activities::age_yearly_mobility_rate[age]) {
      int size = household->get_size();
      for (int i = 0; i < size; i++) {
	Person *p = household->get_housemate(i);
	mobility_moved[p->get_age()]++;
      }
    }
  }


  int popsize = Global::Pop.get_pop_size();
  if (mcount == popsize) {
    double mobility_rate[MAX_MOBILITY_AGE + 1];
    FILE *fp;
    fp = fopen("mobility.out", "w");
    for (int i = 0; i <= MAX_MOBILITY_AGE; i++) {
      mobility_rate[i] = 0;
      if (mobility_count[i]> 0) mobility_rate[i] = (1.0*mobility_moved[i])/mobility_count[i];
      fprintf(fp, "%d %d %d %f\n", i, mobility_count[i], mobility_moved[i], mobility_rate[i]);
    }
    fclose(fp);
  }


}

void Activities::addIncidence(int disease, vector<int> strains) {
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if(favorite_place[i] == NULL) continue;
    favorite_place[i]->modifyIncidenceCount(disease, strains, 1);
  }
}

void Activities::addPrevalence(int disease, vector<int> strains) {
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if(favorite_place[i] == NULL) continue;
    favorite_place[i]->modifyPrevalenceCount(disease, strains, 1);
  }
}


void Activities::read_init_files() {
  Params::get_param((char *) "sick_day_prob", &Activities::sick_day_prob);
  Params::get_param((char *) "community_distance", &Activities::Community_distance);
  Params::get_param((char *) "community_prob", &Activities::Community_prob);
  Params::get_param((char *) "home_neighborhood_prob", &Activities::Home_neighborhood_prob);
  Params::get_param((char *) "unauthorized_sick_leave_prob", &Activities::Unauthorized_sick_leave_prob);

  if (!Global::Enable_Mobility) return;
  char yearly_mobility_rate_file[256];
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read activities init files entered\n"); fflush(Global::Statusfp);
  }
  Params::get_param((char *) "yearly_mobility_rate_file", yearly_mobility_rate_file);
  // read mobility rate file and load the values into the mobility_rate_array
  FILE *fp = fopen(yearly_mobility_rate_file, "r");
  if (fp == NULL) {
    fprintf(Global::Statusfp, "Activities init_file %s not found\n", yearly_mobility_rate_file);
    exit(1);
  }
  for (int i = 0; i <= MAX_MOBILITY_AGE; i++) {
    int age;
    double mobility_rate;
    if (fscanf(fp, "%d %lf",
	       &age, &mobility_rate) != 2) {
      fprintf(Global::Statusfp, "Help! Read failure for age %d\n", i);
      Utils::fred_abort("");
    }
    Activities::age_yearly_mobility_rate[age] = mobility_rate;
  }
  fclose(fp);
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "finished reading Activities init_file = %s\n", yearly_mobility_rate_file);
    for (int i = 0; i <= MAX_MOBILITY_AGE; i++) {
      fprintf(Global::Statusfp, "%d %f\n", i, Activities::age_yearly_mobility_rate[i]);
    }
    fflush(Global::Statusfp);
  }
}


void Activities::terminate() {
  // unenroll from all the favorite places
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (favorite_place[i] != NULL) {
      favorite_place[i]->unenroll(self);
    }
  }
}

void Activities::store_favorite_places() {
  tmp_favorite_place = new Place* [FAVORITE_PLACES];
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    tmp_favorite_place[i] = favorite_place[i];
  }
}

void Activities::restore_favorite_places() {
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    favorite_place[i] = tmp_favorite_place[i];
  }
  delete[] tmp_favorite_place;
}

void Activities::start_traveling(Person * visited) {
  if (visited == NULL) {
    traveling_outside = true;
  }
  else {
    store_favorite_places();
    for (int i = 0; i < FAVORITE_PLACES; i++) {
      favorite_place[i] = NULL;
    }
    favorite_place[HOUSEHOLD_INDEX] = visited->get_household();
    favorite_place[NEIGHBORHOOD_INDEX] = visited->get_neighborhood();
    if (profile == WORKER_PROFILE) {
      favorite_place[WORKPLACE_INDEX] = visited->get_workplace();
      favorite_place[OFFICE_INDEX] = visited->get_office();
    }
  }
  travel_status = true;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "start traveling: id = %d\n", self->get_id());
    fflush(Global::Statusfp);
  }
}

void Activities::stop_traveling() {
  if (!traveling_outside) {
    restore_favorite_places();
  }
  travel_status = false;
  traveling_outside = false;
  if (Global::Verbose > 1) {
    fprintf(Global::Statusfp, "stop traveling: id = %d\n", self->get_id());
    fflush(Global::Statusfp);
  }
}


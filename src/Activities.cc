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
#include "Person.h"
#include "Behavior.h"
#include "Global.h"
#include "Profile.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Params.h"
#include "Vaccine_Manager.h"
#include "Manager.h"
#include "Date.h"
#include "Cell.h"
#include "Grid.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Place_List.h"
#include "Utils.h"
#include "Household.h"

bool Activities::is_initialized = false;
double Activities::age_yearly_mobility_rate[MAX_MOBILITY_AGE + 1];

Activities::Activities (Person *person, Place **fav_place) {
  //Create the static arrays one time
  if (!Activities::is_initialized) {
    read_init_files();
    Activities::is_initialized = true;
  }
  self = person;
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    favorite_place[i] = fav_place[i];
  }
  assert(get_household() != NULL);

  // get the neighbhood from the household
  favorite_place[NEIGHBORHOOD_INDEX] =
    favorite_place[HOUSEHOLD_INDEX]->get_grid_cell()->get_neighborhood();
  if (get_neighborhood() == NULL) {
    printf("Help! NO NEIGHBORHOOD for person %d house %d \n",
      person->get_id(), get_household()->get_id());
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
}

Activities::Activities (Person *person, char *house, char *school, char *work) {
  //Create the static arrays one time
  if (!Activities::is_initialized) {
    read_init_files();
    Activities::is_initialized = true;
  }
  self = person;
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    favorite_place[i] = NULL;
  }
  favorite_place[HOUSEHOLD_INDEX] = Global::Places.get_place_from_label(house);
  favorite_place[SCHOOL_INDEX] = Global::Places.get_place_from_label(school);
  favorite_place[WORKPLACE_INDEX] = Global::Places.get_place_from_label(work);
  assert(get_household() != NULL);

  // get the neighbhood from the household
  favorite_place[NEIGHBORHOOD_INDEX] =
    favorite_place[HOUSEHOLD_INDEX]->get_grid_cell()->get_neighborhood();
  if (get_neighborhood() == NULL) {
    printf("Help! NO NEIGHBORHOOD for person %d house %d \n",
           person->get_id(), get_household()->get_id());
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
  // we avoid computing this except as happens below
  // update_schedule(day);
}

void Activities::update_infectious_activities(Date *sim_start_date, int day) {
  int diseases = self->get_diseases();
  for (int dis = 0; dis < diseases; dis++) {
    if (self->is_infectious(dis)) {
      char status = self->get_disease_status(dis);
      if (schedule_updated < day) update_schedule(sim_start_date, day);
      for (int i = 0; i < FAVORITE_PLACES; i++) {
        if (on_schedule[i]) {
          favorite_place[i]->add_infectious(dis, self, status);
        }
      }
    }
  }
}

void Activities::update_susceptible_activities(Date *sim_start_date, int day) {
  int diseases = self->get_diseases();
  for (int dis = 0; dis < diseases; dis++) {
    if (self->is_susceptible(dis)) {
      for (int i = 0; i < FAVORITE_PLACES; i++) {
        if (favorite_place[i] != NULL && favorite_place[i]->is_infectious(dis)) {
          if (schedule_updated < day) update_schedule(sim_start_date, day);
          if (on_schedule[i]) {
            favorite_place[i]->add_susceptible(dis, self);
          }
        }
      }
    }
  }
}

void Activities::update_schedule(Date *sim_start_date, int day) {
  int day_of_week;
  if (schedule_updated < day) {
    day_of_week = sim_start_date->get_day_of_week(day);
    schedule_updated = day;
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      on_schedule[p] = false;
    }

    // decide whether to stay at home if symptomatic
    int is_symptomatic = self->is_symptomatic();
    if (is_symptomatic) {
      if (RANDOM() < Disease::get_prob_stay_home()) {
        on_schedule[HOUSEHOLD_INDEX] = true;
        return;
      }
    }

    // decide whether to stay home from school
    if (0 < day_of_week && day_of_week < 6 && self->get_age() < Global::ADULT_AGE) {
      if (self->get_behavior()->will_keep_kids_home()) {
        on_schedule[HOUSEHOLD_INDEX] = true;
        return;
      }
    }

    // if not staying home or traveling, consult usual schedule
    for (int p = 0; p < FAVORITE_PLACES; p++) {
      if (favorite_place[p] != NULL)
        on_schedule[p] = is_visited(p, profile, day_of_week);
    }

    if (on_schedule[NEIGHBORHOOD_INDEX]) {
      // pick the neighborhood to visit today
      favorite_place[NEIGHBORHOOD_INDEX] =
        favorite_place[HOUSEHOLD_INDEX]->get_grid_cell()->select_neighborhood();
    }

    // visit classroom or office iff going to school or work
    on_schedule[CLASSROOM_INDEX] = on_schedule[SCHOOL_INDEX];
    on_schedule[OFFICE_INDEX] = on_schedule[WORKPLACE_INDEX];

    if (Global::Verbose > 2) {
      printf("update_schedule on day %d\n", day);
      print_schedule();
      fflush(stdout);
    }
  }
}

void Activities::print_schedule() {
  fprintf(Global::Statusfp, "Schedule for person %d  ", self->get_id());
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (on_schedule[p])
      fprintf(Global::Statusfp, "%d ", favorite_place[p]->get_id());
    else {
      if (favorite_place[p] == NULL) {
        fprintf(Global::Statusfp, "-1 ");
      } else {
        fprintf(Global::Statusfp, "-%d ", favorite_place[p]->get_id());
      }
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

void Activities::update_profile() {
  int age = self->get_age();
  // int old_profile = profile;

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
    if (c != NULL && c->get_age_level() == age) {
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
                  favorite_place[CLASSROOM_INDEX]->get_label());
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
  if (household->get_HoH() == self) {
    if (RANDOM() < Activities::age_yearly_mobility_rate[age]) {
      int size = household->get_size();
      for (int i = 0; i < size; i++) {
	Person *p = household->get_housemate(i);
	mobility_moved[p->get_age()]++;
      }
    }
  }


  int popsize = self->get_population()->get_pop_size();
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
  char yearly_mobility_rate_file[256];
  if (!Global::Enable_Mobility) return;
  if (Global::Verbose) {
    fprintf(Global::Statusfp, "read activities init files entered\n"); fflush(Global::Statusfp);
  }
  get_param((char *) "yearly_mobility_rate_file", yearly_mobility_rate_file);
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
      abort();
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


void Activities::withdraw() {
  // unenroll from all the favorite places
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (favorite_place[i] != NULL) {
      favorite_place[i]->unenroll(self);
    }
  }
}


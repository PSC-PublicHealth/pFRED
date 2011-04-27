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
#include "Patch.h"
#include "School.h"
#include "Classroom.h"
#include "Workplace.h"
#include "Grid.h"

Activities::Activities (Person *person, Place **fav_place) {
  self = person;

  for (int i = 0; i < FAVORITE_PLACES; i++) {
    favorite_place[i] = fav_place[i];
    }

  assert(get_household() != NULL);

  // get the neighbhood from the household
  favorite_place[NEIGHBORHOOD_INDEX] =
    favorite_place[HOUSEHOLD_INDEX]->get_patch()->get_neighborhood();

  if (get_neighborhood() == NULL) {
    printf("Help! NO NEIGHBORHOOD for person %d house %d \n",
           person->get_id(), get_household()->get_id());
    }

  assert(get_neighborhood() != NULL);
  assign_profile();
  schedule_updated = -1;
  }

void Activities::assign_profile() {
  int age = self->get_age();

  if (age < SCHOOL_AGE && get_school() == NULL)
    profile = PRESCHOOL_PROFILE;    // child at home
  else if (age < SCHOOL_AGE && get_school() != NULL)
    profile = STUDENT_PROFILE;      // child in preschool
  else if (age < ADULT_AGE && get_school() != NULL)
    profile = STUDENT_PROFILE;      // student
  else if (get_school() != NULL)
    profile = TEACHER_PROFILE;      // teacher
  else if (RETIREMENT_AGE <= age && RANDOM() < 0.5)
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

void Activities::reset() {
  // enroll in all the favorite places
  for (int i = 0; i < FAVORITE_PLACES; i++) {
    if (favorite_place[i] != NULL) {
      favorite_place[i]->enroll(self);
      }
    }

  // reset the daily schedule
  schedule_updated = -1;
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
    if (0 < day_of_week && day_of_week < 6 && self->get_age() < ADULT_AGE) {
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
        favorite_place[HOUSEHOLD_INDEX]->get_patch()->select_neighborhood();
      }

    // visit classroom or office iff going to school or work
    on_schedule[CLASSROOM_INDEX] = on_schedule[SCHOOL_INDEX];
    on_schedule[OFFICE_INDEX] = on_schedule[WORKPLACE_INDEX];

    if (Verbose > 2) {
      printf("update_schedule on day %d\n", day);
      print_schedule();
      fflush(stdout);
      }
    }
  }

void Activities::print_schedule() {
  fprintf(Statusfp, "Schedule for person %d  ", self->get_id());

  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (on_schedule[p])
      fprintf(Statusfp, "%d ", favorite_place[p]->get_id());
    else {
      if (favorite_place[p] == NULL) {
        fprintf(Statusfp, "-1 ");
        }
      else {
        fprintf(Statusfp, "-%d ", favorite_place[p]->get_id());
        }
      }
    }

  fprintf(Statusfp, "\n");
  fflush(Statusfp);
  }

void Activities::print() {
  fprintf(Statusfp, "Activities for person %d: ", self->get_id());

  for (int p = 0; p < FAVORITE_PLACES; p++) {
    if (favorite_place[p] != NULL)
      fprintf(Statusfp, "%d ", favorite_place[p]->get_id());
    else {
      fprintf(Statusfp, "-1 ");
      }
    }

  fprintf(Statusfp, "\n");
  fflush(Statusfp);
  }

void Activities::assign_school() {
  int age = self->get_age();

  if (age < SCHOOL_AGE || ADULT_AGE <= age) return;

  Patch *patch = favorite_place[HOUSEHOLD_INDEX]->get_patch();
  assert(patch != NULL);
  Place *p = patch->select_random_school(age);

  if (p != NULL) {
    favorite_place[SCHOOL_INDEX] = p;
    favorite_place[CLASSROOM_INDEX] = NULL;
    assign_classroom();
    return;
    }

  int trials = 0;

  while (trials < 100) {
    patch = Environment.select_random_patch();
    p = patch->select_random_school(age);

    if (p != NULL) {
      favorite_place[SCHOOL_INDEX] = p;
      favorite_place[CLASSROOM_INDEX] = NULL;
      assign_classroom();
      return;
      }

    trials++;
    }

  fprintf(Statusfp, "assign_school: can't locate school for person %d\n",
          self->get_id());
  abort();
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

  if (age < ADULT_AGE) return;

  Patch *patch = favorite_place[HOUSEHOLD_INDEX]->get_patch();
  assert(patch != NULL);
  Place *p = patch->select_random_workplace();

  if (p != NULL) {
    favorite_place[WORKPLACE_INDEX] = p;
    favorite_place[OFFICE_INDEX] = NULL;
    assign_office();
    return;
    }

  int trials = 0;

  while (trials < 100) {
    patch = Environment.select_random_patch();
    p = patch->select_random_workplace();

    if (p != NULL) {
      favorite_place[WORKPLACE_INDEX] = p;
      favorite_place[OFFICE_INDEX] = NULL;
      assign_office();
      return;
      }

    trials++;
    }

  fprintf(Statusfp, "assign_workplace: can't locate workplace for person %d\n",
          self->get_id());
  abort();
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

  if (profile == PRESCHOOL_PROFILE && SCHOOL_AGE <= age && age < ADULT_AGE) {
    // start school
    profile = STUDENT_PROFILE;
    // select a school based on age and neighborhood
    assign_school();

    if (Verbose>1) {
      fprintf(Statusfp,
              "changed behavior profile to STUDENT: id %d age %d sex %c\n",
              self->get_id(), age, self->get_sex());
      print();
      fflush(Statusfp);
      }

    return;
    }

  if (profile == STUDENT_PROFILE && age < ADULT_AGE) {
    // select a school based on age and neighborhood
    School * s = (School *) favorite_place[SCHOOL_INDEX];
    Classroom * c = (Classroom *) favorite_place[CLASSROOM_INDEX];

    if (c != NULL && c->get_age_level() == age) {
      // no change
      if (Verbose>1) {
        fprintf(Statusfp,
                "KEPT CLASSROOM ASSIGNMENT: id %d age %d sex %c ",
                self->get_id(), age, self->get_sex());
        fprintf(Statusfp, "%s %s | ",
                s->get_label(), c->get_label());
        print();
        fflush(Statusfp);
        }

      return;
      }
    else if (s != NULL && s->classrooms_for_age(age) > 0) {
      // pick a new classrooms in current school
      favorite_place[CLASSROOM_INDEX] = NULL;
      assign_classroom();

      if (Verbose>1) {
        fprintf(Statusfp,
                "CHANGED CLASSROOM ASSIGNMENT: id %d age %d sex %c ",
                self->get_id(), age, self->get_sex());

        if (s != NULL && c != NULL) {
          fprintf(Statusfp, "from %s %s to %s %s | ",
                  s->get_label(), c->get_label(),
                  favorite_place[SCHOOL_INDEX]->get_label(),
                  favorite_place[CLASSROOM_INDEX]->get_label());
          }
        else {
          fprintf(Statusfp, "from NULL NULL to %s %s | ",
                  favorite_place[SCHOOL_INDEX]->get_label(),
                  favorite_place[CLASSROOM_INDEX]->get_label());
          }

        print();
        fflush(Statusfp);
        }
      }
    else {
      // pick a new school and classroom
      assign_school();

      if (Verbose>1) {
        fprintf(Statusfp,
                "CHANGED SCHOOL ASSIGNMENT: id %d age %d sex %c ",
                self->get_id(), age, self->get_sex());

        if (s != NULL && c != NULL) {
          fprintf(Statusfp, "from %s %s to %s %s | ",
                  s->get_label(), c->get_label(),
                  favorite_place[SCHOOL_INDEX]->get_label(),
                  favorite_place[CLASSROOM_INDEX]->get_label());
          }
        else {
          fprintf(Statusfp, "from NULL NULL to %s %s | ",
                  favorite_place[SCHOOL_INDEX]->get_label(),
                  favorite_place[CLASSROOM_INDEX]->get_label());
          }

        print();
        fflush(Statusfp);
        }
      }

    return;
    }

  if (profile == STUDENT_PROFILE && ADULT_AGE <= age) {
    // leave school
    favorite_place[SCHOOL_INDEX] = NULL;
    favorite_place[CLASSROOM_INDEX] = NULL;
    // get a job
    profile = WORKER_PROFILE;
    assign_workplace();

    if (Verbose>1) {
      fprintf(Statusfp,
              "changed behavior profile to WORKER: id %d age %d sex %c\n",
              self->get_id(), age, self->get_sex());
      print();
      fflush(Statusfp);
      }

    return;
    }

  if (profile != RETIRED_PROFILE && RETIREMENT_AGE <= age) {
    if (RANDOM() < 0.5) {
      // quit working
      favorite_place[WORKPLACE_INDEX] = NULL;
      favorite_place[OFFICE_INDEX] = NULL;
      profile = RETIRED_PROFILE;

      if (Verbose>1) {
        fprintf(Statusfp,
                "changed behavior profile to RETIRED: age %d age %d sex %c\n",
                self->get_id(), age, self->get_sex());
        print();
        fflush(Statusfp);
        }
      }

    return;
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

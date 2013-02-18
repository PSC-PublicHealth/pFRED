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

bool Activities::is_initialized = false;
bool Activities::is_weekday = false;
double Activities::age_yearly_mobility_rate[MAX_MOBILITY_AGE + 1];
int Activities::day_of_week = 0;

// run-time parameters
double Activities::Community_distance = 20;
double Activities::Community_prob = 0.1;
double Activities::Home_neighborhood_prob = 0.5;
int Activities::Enable_default_sick_behavior = 0;
double Activities::Default_sick_day_prob = 0.0;
double Activities::SLA_mean_sick_days_absent = 0.0;
double Activities::SLU_mean_sick_days_absent = 0.0;
double Activities::SLA_absent_prob = 0.0;
double Activities::SLU_absent_prob = 0.0;
double Activities::Flu_days = 0.0;

// sick leave statistics
int Activities::Sick_days_present = 0;
int Activities::Sick_days_absent = 0;
int Activities::School_sick_days_present = 0;
int Activities::School_sick_days_absent = 0;

Activities::Activities (Person * self, Place *house, Place *school, Place *work) {
  //Create the static arrays one time
  if (!Activities::is_initialized) {
    read_init_files();
    Activities::is_initialized = true;
  }
  clear_favorite_places();
  set_household(house);
  set_school(school);
  set_workplace(work);
  assert(get_household() != NULL);

  // get the neighbhood from the household
  set_neighborhood(get_household()->get_grid_cell()->get_neighborhood());
  FRED_CONDITIONAL_VERBOSE( 0, get_neighborhood() == NULL,
      "Help! NO NEIGHBORHOOD for person %d house %d \n",
      self->get_id(), get_household()->get_id());
  assert(get_neighborhood() != NULL);
  home_neighborhood = get_neighborhood();

  // assign profiles and enroll in favorite places
  assign_profile( self );
  enroll_in_favorite_places( self );

  // need to set the daily schedule
  schedule_updated = -1;
  travel_status = false;
  traveling_outside = false;

  // sick leave variables
  my_sick_days_absent = 0;
  my_sick_days_present = 0;
  my_sick_leave_decision_has_been_made = false;
  my_sick_leave_decision = false;
  sick_days_remaining = 0.0;
  sick_leave_available = false;
}

void Activities::prepare() {
  initialize_sick_leave();
}

#define SMALL_COMPANY_MAXSIZE 49
#define MID_COMPANY_MAXSIZE 99
#define MEDIUM_COMPANY_MAXSIZE 499

static int employees_small_with_sick_leave = 0;
static int employees_small_without_sick_leave = 0;
static int employees_med_with_sick_leave = 0;
static int employees_med_without_sick_leave = 0;
static int employees_large_with_sick_leave = 0;
static int employees_large_without_sick_leave = 0;
static int employees_xlarge_with_sick_leave = 0;
static int employees_xlarge_without_sick_leave = 0;

void Activities::initialize_sick_leave() {
  int workplace_size = 0;
  my_sick_days_absent = 0;
  my_sick_days_present = 0;
  my_sick_leave_decision_has_been_made = false;
  my_sick_leave_decision = false;
  sick_days_remaining = 0.0;
  sick_leave_available = false;

  if (get_workplace() != NULL)
    workplace_size = get_workplace()->get_size();
  else {
    if (is_teacher()) {
      workplace_size = ((School *) get_school())->get_staff_size();
    }
  }

  // is sick leave available?
  if (workplace_size > 0) {
    if (workplace_size <= SMALL_COMPANY_MAXSIZE) {
      sick_leave_available = (RANDOM() < 0.53);
      if (sick_leave_available)
        employees_small_with_sick_leave++;
      else
        employees_small_without_sick_leave++;
    }
    else if (workplace_size <= MID_COMPANY_MAXSIZE) {
      sick_leave_available = (RANDOM() < 0.58);
      if (sick_leave_available)
        employees_med_with_sick_leave++;
      else
        employees_med_without_sick_leave++;
    }
    else if (workplace_size <= MEDIUM_COMPANY_MAXSIZE) {
      sick_leave_available = (RANDOM() < 0.70);
      if (sick_leave_available)
        employees_large_with_sick_leave++;
      else
        employees_large_without_sick_leave++;
    }
    else {
      sick_leave_available = (RANDOM() < 0.85);
      if (sick_leave_available)
        employees_xlarge_with_sick_leave++;
      else
        employees_xlarge_without_sick_leave++;
    }
  }
  else
    sick_leave_available = false;

  // compute sick days remaining (for flu)
  sick_days_remaining = 0.0;
  if (sick_leave_available) {
    if (RANDOM() < Activities::SLA_absent_prob) {
      sick_days_remaining = Activities::SLA_mean_sick_days_absent + Activities::Flu_days;
    }
  }
  else {
    if (RANDOM() < Activities::SLU_absent_prob) {
      sick_days_remaining = Activities::SLU_mean_sick_days_absent + Activities::Flu_days;
    }
    else if (RANDOM() < Activities::SLA_absent_prob - Activities::SLU_absent_prob) {
      sick_days_remaining = Activities::Flu_days;
    }
  }
}

void Activities::before_run() {
    FRED_STATUS( 0, "employees at small workplaces with sick leave: %d\n",
        employees_small_with_sick_leave );
    FRED_STATUS( 0, "employees at small workplaces without sick leave: %d\n",
        employees_small_without_sick_leave );
    FRED_STATUS( 0, "employees at med workplaces with sick leave: %d\n",
        employees_med_with_sick_leave );
    FRED_STATUS( 0, "employees at med workplaces without sick leave: %d\n",
        employees_med_without_sick_leave );
    FRED_STATUS( 0, "employees at large workplaces with sick leave: %d\n",
        employees_large_with_sick_leave );
    FRED_STATUS( 0, "employees at large workplaces without sick leave: %d\n",
        employees_large_without_sick_leave );
    FRED_STATUS( 0, "employees at xlarge workplaces with sick leave: %d\n",
        employees_xlarge_with_sick_leave );
    FRED_STATUS( 0, "employees at xlarge workplaces without sick leave: %d\n",
        employees_xlarge_without_sick_leave );
}

void Activities::assign_profile( Person * self ) {
  profile = UNDEFINED_PROFILE;
  int age = self->get_age();
  if (age < Global::SCHOOL_AGE && get_school() == NULL)
    profile = PRESCHOOL_PROFILE;    // child at home
  else if (age < Global::SCHOOL_AGE && get_school() != NULL)
    profile = STUDENT_PROFILE;      // child in preschool
  else if (get_school() != NULL)
    profile = STUDENT_PROFILE;      // student
  // else if (get_school() != NULL)
  // profile = TEACHER_PROFILE;      // teacher
  else if (Global::RETIREMENT_AGE <= age && RANDOM() < 0.5)
    profile = RETIRED_PROFILE;      // retired
  else if (get_workplace() == NULL)
    profile = UNEMPLOYED_PROFILE;
  else
    profile = WORKER_PROFILE;     // worker

  // weekend worker
  if (profile == WORKER_PROFILE && RANDOM() < 0.2) {
    profile = WEEKEND_WORKER_PROFILE;   // 20% weekend worker
  }
}

void Activities::update(int day) {

  FRED_STATUS( 1, "Activities update entered\n");

  // decide if this is a weekday:
  Activities::day_of_week = Global::Sim_Current_Date->get_day_of_week();
  Activities::is_weekday = (0 < Activities::day_of_week && Activities::day_of_week < 6);

  // print out absenteeism/presenteeism counts
  FRED_CONDITIONAL_VERBOSE( 0, day > 0,
      "DAY %d ABSENTEEISM: work absent %d present %d %0.2f  school absent %d present %d %0.2f\n", day-1,
      Activities::Sick_days_absent,
      Activities::Sick_days_present,
      (double) (Activities::Sick_days_absent) /(double)(1+Activities::Sick_days_absent+Activities::Sick_days_present),
      Activities::School_sick_days_absent,
      Activities::School_sick_days_present,
      (double) (Activities::School_sick_days_absent) /(double)(1+Activities::School_sick_days_absent+Activities::School_sick_days_present));

  // keep track of global activity counts
  Activities::Sick_days_present = 0;
  Activities::Sick_days_absent = 0;
  Activities::School_sick_days_present = 0;
  Activities::School_sick_days_absent = 0;

  FRED_STATUS( 1, "Activities update completed\n");
}


void Activities::update_infectious_activities( Person * self, int day, int dis ) {
  // skip scheduled activities if traveling abroad
  if (traveling_outside)
    return;

  if ( ! self->is_infectious( dis ) ) {
    return;
  }
  // get list of places to visit today
  update_schedule( self, day );

  // if symptomatic, decide whether or not to stay home
  if (self->is_symptomatic()) {
    decide_whether_to_stay_home( self, day );
  }

  make_favorite_places_infectious( self, dis );
}


void Activities::update_susceptible_activities( Person * self, int day, int dis ) {
  // skip scheduled activities if traveling abroad
  if (traveling_outside)
    return;

  // get list of places to visit today
  update_schedule( self, day );

  if ( on_schedule.any() ) {
    join_susceptible_lists_at_favorite_places(self, dis);
  }
}

void Activities::update_schedule( Person * self, int day ) {
  // update this schedule only once per day
  if (day <= schedule_updated)
    return;
  schedule_updated = day;
  on_schedule.reset();
  // always visit the household
  on_schedule[HOUSEHOLD_ACTIVITY] = true;
  // provisionally visit the neighborhood
  on_schedule[NEIGHBORHOOD_ACTIVITY] = true;
  // weekday vs weekend provisional activity
  if (Activities::is_weekday) {
    if (get_school() != NULL)
      on_schedule[SCHOOL_ACTIVITY] = true;
    if (get_classroom() != NULL)
      on_schedule[CLASSROOM_ACTIVITY] = true;
    if (get_workplace() != NULL)
      on_schedule[WORKPLACE_ACTIVITY] = true;
    if (get_office() != NULL)
      on_schedule[OFFICE_ACTIVITY] = true;
  }
  else {
    if (profile == WEEKEND_WORKER_PROFILE || profile == STUDENT_PROFILE) {
      if (get_workplace() != NULL)
        on_schedule[WORKPLACE_ACTIVITY] = true;
      if (get_office() != NULL)
        on_schedule[OFFICE_ACTIVITY] = true;
    }
  }
  // skip work at background absenteeism rate
  if (Global::Work_absenteeism > 0.0 && on_schedule[WORKPLACE_ACTIVITY]) {
    if (RANDOM() < Global::Work_absenteeism) {
      on_schedule[WORKPLACE_ACTIVITY] = false;
      on_schedule[OFFICE_ACTIVITY] = false;
    }
  }
  // skip school at background school absenteeism rate
  if (Global::School_absenteeism > 0.0 && on_schedule[SCHOOL_ACTIVITY]) {
    if (RANDOM() < Global::School_absenteeism) {
      on_schedule[SCHOOL_ACTIVITY] = false;
      on_schedule[CLASSROOM_ACTIVITY] = false;
    }
  }
  // decide whether to stay home if symptomatic moved to update_infectious_activities
  // decide which neighborhood to visit today
  if (on_schedule[NEIGHBORHOOD_ACTIVITY]) {
    double r = RANDOM();
    if ( r < Home_neighborhood_prob ) {
      set_neighborhood(home_neighborhood);
    }
    else {
      set_neighborhood(((Household *)get_household())->select_new_neighborhood( Community_prob, Community_distance, Home_neighborhood_prob, r ));
    }
  }
  FRED_STATUS( 1, "update_schedule on day %d\n%s\n",
      day, schedule_to_string( self, day ).c_str() );
}

void Activities::decide_whether_to_stay_home( Person * self, int day ) {
  assert (self->is_symptomatic());
  bool stay_home = false;
  bool it_is_a_workday = (on_schedule[WORKPLACE_ACTIVITY] || (is_teacher() && on_schedule[SCHOOL_ACTIVITY]));

  if (self->is_adult()) {
    if (Activities::Enable_default_sick_behavior) {
      stay_home = default_sick_leave_behavior();
    }
    else {
      if (it_is_a_workday) {
        // it is a work day
        if (sick_days_remaining > 0.0) {
          stay_home = (RANDOM() < sick_days_remaining);
          sick_days_remaining--;
        }
        else {
          stay_home = false;
        }
      }
      else {
        // it is a not work day
        stay_home = (RANDOM() < Activities::Default_sick_day_prob);
      }
    }
  }
  else {
    // sick child: use default sick behavior, for now.
    stay_home = default_sick_leave_behavior();
  }

  // record work absent/present decision if it is a workday
  if (it_is_a_workday) {
    if (stay_home) {
      Activities::Sick_days_absent++;
      my_sick_days_absent++;
    }
    else {
      Activities::Sick_days_present++;
      my_sick_days_present++;
    }
  }

  // record school absent/present decision if it is a school day
  if ((is_teacher()==false) && on_schedule[SCHOOL_ACTIVITY]) {
    if (stay_home) {
      Activities::School_sick_days_absent++;
      my_sick_days_absent++;
    }
    else {
      Activities::School_sick_days_present++;
      my_sick_days_present++;
    }
  }

  if (stay_home) {
    // withdraw to household
    on_schedule[WORKPLACE_ACTIVITY] = false;
    on_schedule[OFFICE_ACTIVITY] = false;
    on_schedule[SCHOOL_ACTIVITY] = false;
    on_schedule[CLASSROOM_ACTIVITY] = false;
    on_schedule[NEIGHBORHOOD_ACTIVITY] = false;
    // printf("agent %d staying home on day %d\n", self->get_id(), day);
  }
}

bool Activities::default_sick_leave_behavior() {
  bool stay_home = false;
  if (my_sick_leave_decision_has_been_made) {
    stay_home = my_sick_leave_decision;
  }
  else {
    stay_home = (RANDOM() < Activities::Default_sick_day_prob);
    my_sick_leave_decision = stay_home;
    my_sick_leave_decision_has_been_made = true;
  }
  return stay_home;
}

void Activities::print_schedule( Person * self, int day ) {
  FRED_STATUS( 0, "%s\n", schedule_to_string( self, day ).c_str() );
}

void Activities::print( Person * self ) {
  FRED_STATUS( 0, "%s\n", to_string( self ).c_str() );
}

void Activities::assign_school( Person * self ) {
  int age = self->get_age();
  // if (age < Global::SCHOOL_AGE || Global::ADULT_AGE <= age) return;
  Cell *grid_cell = get_household()->get_grid_cell();
  assert(grid_cell != NULL);
  Place *p = grid_cell->select_random_school(age);
  if (p != NULL) {
    set_school(p);
    set_classroom(NULL);
    assign_classroom( self );
    return;
  }
  else {
    set_school(NULL);
    set_classroom(NULL);
    return;
  }

  int src_pop = grid_cell->get_target_popsize();
  int row = grid_cell->get_row();
  int col = grid_cell->get_col();
  int level = 1;
  vector<Cell *> nbrs;
  while(level < 100) {
    nbrs.clear();
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row-level, col+j) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+j, col+level) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+level, col+j) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+j, col-level) );

    int target_pop = 0;
    for(unsigned int i=0; i<nbrs.size(); i++) {
      target_pop += nbrs.at(i)->get_target_popsize();
    }
    // Radiation model (Simini et al, 2011)
    // Approximated to concentric squares instead of concentric circles
    double prob = 1.0 / (1.0 + ((double) target_pop) / src_pop);
    double prob_per_person = prob / target_pop;

    int attempts = 0;
    for(unsigned int i=0; i<nbrs.size(); i++) {
      Cell *nbr = nbrs.at(i);
      if(nbr == NULL) continue;
      else attempts++;
      double r = RANDOM();
      r = 0;
      if(r < prob_per_person * nbr->get_target_popsize()) {
        p = nbr->select_random_school(age);
        if(p != NULL) {
          set_school(p);
	  set_classroom(NULL);
          assign_classroom( self );
          return;
        }
      }
    }
    if(attempts == 0) break;
    level++;
  }

  /*
     int trials = 0;
     while (trials < 100) {
     grid_cell = (Cell *) Global::Cells->select_random_grid_cell();
     p = grid_cell->select_random_school(age);
     if (p != NULL) {
     set_school(p);
     set_classroom(NULL);
     assign_classroom();

     return;
     }
     trials++;
     }*/
  Utils::fred_abort("assign_school: can't locate school for person %d\n", self->get_id());
  //FRED_WARNING("assign_school: can't locate school for person %d\n", self->get_id());
}

void Activities::assign_classroom( Person * self ) {
  if (get_school() != NULL &&
      get_classroom() == NULL) {
    Place * place = ((School *)get_school())->select_classroom_for_student(self);
    if (place != NULL) place->enroll(self);
    set_classroom(place);
  }
}

void Activities::assign_workplace( Person * self ) {
  int age = self->get_age();
  Cell *grid_cell = get_household()->get_grid_cell();
  assert(grid_cell != NULL);
  Place *p = grid_cell->select_random_workplace();
  if (p != NULL) {
    set_workplace(p);
    set_office(NULL);
    assign_office( self );
    return;
  }
  else {
    set_workplace(NULL);
    set_office(NULL);
    return;
  }


  int src_pop = grid_cell->get_target_popsize();
  int row = grid_cell->get_row();
  int col = grid_cell->get_col();
  int level = 1;
  vector<Cell *> nbrs;
  while(level < 100) {
    nbrs.clear();
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row-level, col+j) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+j, col+level) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+level, col+j) );
    for(int j=-level; j<level; j++)
      nbrs.push_back( Global::Cells->get_grid_cell(row+j, col-level) );

    int target_pop = 0;
    for(unsigned int i=0; i<nbrs.size(); i++) {
      target_pop += nbrs.at(i)->get_target_popsize();
    }
    // Radiation model (Simini et al, 2011)
    // Approximated to concentric squares instead of concentric circles
    double prob = 1.0 / (1.0 + ((double) target_pop) / src_pop);
    double prob_per_person = prob / target_pop;

    int attempts = 0;
    for(unsigned int i=0; i<nbrs.size(); i++) {
      Cell *nbr = nbrs.at(i);
      if(nbr == NULL) continue;
      else attempts++;
      double r = RANDOM();
      r = 0;
      if(r < prob_per_person * nbr->get_target_popsize()) {
        p = nbr->select_random_workplace();
        if(p != NULL) {
          set_workplace(p);
          set_office(NULL);
          assign_office( self );
          return;
        }
      }
    }
    if(attempts == 0) break;
    level++;
  }

  /*
     int trials = 0;
     while (trials < 100) {
     grid_cell = (Cell *) Global::Cells->select_random_grid_cell();
     p = grid_cell->select_random_workplace();
     if (p != NULL) {
     set_workplace(p);
     set_office(NULL);
     assign_office();
     return;
     }
     trials++;
     }*/
  //Utils::fred_abort("assign_workplace: can't locate workplace for person %d\n", self->get_id());
  FRED_WARNING("assign_workplace: can't locate workplace for person %d\n", self->get_id());
}

void Activities::assign_office( Person * self ) {
  if (get_workplace() != NULL &&
      get_office() == NULL &&
      Workplace::get_max_office_size() > 0) {
    Place * place =
      ((Workplace *) get_workplace())->assign_office(self);
    if (place != NULL) place->enroll(self);
    else { 
      FRED_VERBOSE( 0, "Warning! No office assigned for person %d workplace %d\n",
		    self->get_id(), get_workplace()->get_id());
    }
    set_office(place);
  }
}

int Activities::get_degree() {
  int degree;
  int n;
  degree = 0;
  n = get_group_size(NEIGHBORHOOD_ACTIVITY);
  if (n > 0) degree += (n-1);
  n = get_group_size(SCHOOL_ACTIVITY);
  if (n > 0) degree += (n-1);
  n = get_group_size(WORKPLACE_ACTIVITY);
  if (n > 0) degree += (n-1);
  return degree;
}

void Activities::update_profile( Person * self ) {
  int age = self->get_age();

  if (profile == PRESCHOOL_PROFILE && Global::SCHOOL_AGE <= age && age < Global::ADULT_AGE) {
    // start school
    profile = STUDENT_PROFILE;
    // select a school based on age and neighborhood
    assign_school( self );
    
    FRED_STATUS( 1, "changed behavior profile to STUDENT: id %d age %d sex %c\n%s\n",
          self->get_id(), age, self->get_sex(), to_string( self ).c_str() );
    
    return;
  }

  if ( profile == STUDENT_PROFILE && age < Global::ADULT_AGE ) {
    // select a school based on age and neighborhood
    School * s = (School *) get_school();
    Classroom * c = (Classroom *) get_classroom();
    if (c == NULL || c->get_age_level() == age) {
      // no change
      FRED_STATUS( 1, "KEPT CLASSROOM ASSIGNMENT: id %d age %d sex %c %s %s | %s\n",
        self->get_id(), age, self->get_sex(), s->get_label(), c->get_label(),
        to_string( self ).c_str() );
    }
    else {
      if ( s != NULL && s->classrooms_for_age( age ) > 0 ) {
        // pick a new classrooms in current school
        set_classroom(NULL);
        assign_classroom( self );
      }
      else {
        assign_school( self );
      }
      FRED_STATUS( 1, "CHANGED CLASSROOM ASSIGNMENT: id %d age %d sex %c from %s %s to %s %s | %s\n",
        self->get_id(), age, self->get_sex(),
        s != NULL ? s->get_label() : "NULL",
        c != NULL ? c->get_label() : "NULL",
		   get_school()->get_label(),
		   get_classroom()->get_label(),
        to_string( self ).c_str() );
    }
    return;
  }

  if ( profile == STUDENT_PROFILE && Global::ADULT_AGE <= age ) {
    // leave school
    set_school(NULL);
    set_classroom(NULL);
    // get a job
    profile = WORKER_PROFILE;
    assign_workplace( self );
    initialize_sick_leave();
    FRED_STATUS( 1, "changed behavior profile to WORKER: id %d age %d sex %c\n%s\n",
      self->get_id(), age, self->get_sex(), to_string( self ).c_str() );
    return;
  }

  if (profile != RETIRED_PROFILE && Global::RETIREMENT_AGE <= age) {
    if ( RANDOM() < 0.5 ) {
      // quit working
      if (is_teacher()) {
	set_school(NULL);
	set_classroom(NULL);
      }
      set_workplace(NULL);
      set_office(NULL);
      profile = RETIRED_PROFILE;
      initialize_sick_leave(); // no sick leave available if retired
      FRED_STATUS( 1, "changed behavior profile to RETIRED: age %d age %d sex %c\n%s\n",
        self->get_id(), age, self->get_sex(), to_string( self ).c_str() );
    }
    return;
  }
}

static int mobility_count[MAX_MOBILITY_AGE + 1];
static int mobility_moved[MAX_MOBILITY_AGE + 1];
static int mcount = 0;

void Activities::update_household_mobility( Person * self ) {
  if (!Global::Enable_Mobility) return;

  FRED_STATUS( 1, "update_household_mobility entered with mcount = %d\n", mcount );

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

void Activities::read_init_files() {
  Params::get_param_from_string("community_distance", &Activities::Community_distance);
  Params::get_param_from_string("community_prob", &Activities::Community_prob);
  Params::get_param_from_string("home_neighborhood_prob", &Activities::Home_neighborhood_prob);

  Params::get_param_from_string("enable_default_sick_behavior", &Activities::Enable_default_sick_behavior);
  Params::get_param_from_string("sick_day_prob", &Activities::Default_sick_day_prob);

  Params::get_param_from_string("SLA_mean_sick_days_absent", &Activities::SLA_mean_sick_days_absent);
  Params::get_param_from_string("SLU_mean_sick_days_absent", &Activities::SLU_mean_sick_days_absent);
  Params::get_param_from_string("SLA_absent_prob", &Activities::SLA_absent_prob);
  Params::get_param_from_string("SLU_absent_prob", &Activities::SLU_absent_prob);
  Params::get_param_from_string("flu_days", &Activities::Flu_days);

  if (!Global::Enable_Mobility) return;

  char yearly_mobility_rate_file[FRED_STRING_SIZE];

  FRED_STATUS( 0, "read activities init files entered\n", "" );
  
  Params::get_param_from_string("yearly_mobility_rate_file", yearly_mobility_rate_file);
  // read mobility rate file and load the values into the mobility_rate_array
  FILE *fp = Utils::fred_open_file(yearly_mobility_rate_file);
  if ( fp == NULL ) {
    Utils::fred_abort( "Activities init_file %s not found\n", yearly_mobility_rate_file );
  }
  for (int i = 0; i <= MAX_MOBILITY_AGE; i++) {
    int age;
    double mobility_rate;
    if ( fscanf(fp, "%d %lf", &age, &mobility_rate) != 2 ) {
      Utils::fred_abort( "Help! Read failure for age %d\n", i);
    }
    Activities::age_yearly_mobility_rate[age] = mobility_rate;
  }
  fclose(fp);
  FRED_STATUS( 0, "finished reading Activities init_file = %s\n", yearly_mobility_rate_file);
  for (int i = 0; Global::Verbose && i <= MAX_MOBILITY_AGE; i++) {
    FRED_STATUS( 0, "%d %f\n", i, Activities::age_yearly_mobility_rate[i]);
  }
}

void Activities::start_traveling( Person * self, Person * visited ) {
  if (visited == NULL) {
    traveling_outside = true;
  }
  else {
    store_favorite_places();
    clear_favorite_places();
    set_household(visited->get_household());
    set_neighborhood(visited->get_neighborhood());
    if (profile == WORKER_PROFILE) {
      set_workplace(visited->get_workplace());
      set_office(visited->get_office());
    }
  }
  travel_status = true;
  FRED_STATUS( 1, "start traveling: id = %d\n", self->get_id() );
}

void Activities::stop_traveling( Person * self ) {
  if (!traveling_outside) {
    restore_favorite_places();
  }
  travel_status = false;
  traveling_outside = false;
  FRED_STATUS( 1, "stop traveling: id = %d\n", self->get_id() );
}

bool Activities::become_a_teacher( Person * self, Place *school) {
  bool success = false;
  FRED_STATUS( 1, "Become a teacher entered for person %d age %d\n", self->get_id(), self->get_age());
  if (get_school() != NULL) {
    if (Global::Verbose > 1) {
      FRED_WARNING("become_a_teacher: person %d already goes to school %d age %d\n",
		   self->get_id(), get_school()->get_id(), self->get_age());
    }
    profile = STUDENT_PROFILE;
  }
  else {
    // set profile
    profile = TEACHER_PROFILE;
    // join the school
    FRED_STATUS( 1, "set school to %s\n", school->get_label());
    set_school(school);
    get_school()->enroll(self);
    success = true;
  }

  // withdraw from this workplace and any associated office
  if (get_workplace() != NULL) {
    get_workplace()->unenroll(self);
    FRED_STATUS( 1, "set workplace to NULL\n");
    set_workplace(NULL);
  }
  if (get_office() != NULL) {
    get_office()->unenroll(self);
    set_office(NULL);
  }
  FRED_STATUS( 1, "Become a teacher finished for person %d age %d\n", self->get_id(), self->get_age());
  return success;
}

std::string Activities::schedule_to_string( Person * self, int day ) {
  std::stringstream ss;
  ss << "day " << day << " schedule for person " << self->get_id() << "  ";
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    ss << on_schedule[p]? "+" : "-";
    ss << get_place_id(p) << " ";
  }
  return ss.str(); 
}

std::string Activities::to_string() {
  std::stringstream ss;
  ss <<  "Activities: "; 
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    ss << get_place_label(p) << " ";
  }
  return ss.str();
}
  
std::string Activities::to_string( Person * self ) {
  std::stringstream ss;
  ss <<  "Activities for person " << self->get_id() << ": "; 
  for (int p = 0; p < FAVORITE_PLACES; p++) {
    ss << get_place_id(p) << " ";
  }
  return ss.str();
}
  
unsigned char Activities::get_deme_id() {
  Place * p;
  if ( traveling_outside ) {
    p = get_temporary_household();
  }
  else {
    p = get_household();
  }
  assert( p->is_household() );
  return static_cast< Household * >( p )->get_deme_id();
}

void Activities::terminate( Person * self ) {
  // Person was enrolled in only his original 
  // favorite places, not his host's places while travelling
  if(travel_status && ! traveling_outside) 
    restore_favorite_places();

  unenroll_from_favorite_places(self);
}

void Activities::end_of_run() {
}

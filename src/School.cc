/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: School.cc
//

#include "School.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Disease.h"
#include "Place_List.h"
#include "Classroom.h"
#include "Date.h"

double * school_contacts_per_day;
double *** school_contact_prob;
char school_closure_policy[80];
int school_closure_day;
double school_closure_threshold;
int school_closure_period;
int school_closure_delay;
int school_classroom_size;
int school_parameters_set = 0;
int school_summer_schedule;
char school_summer_start[8];
char school_summer_end[8];

School::School(int loc, const char *lab, double lon, double lat, Place* container, Population *pop) {
  type = SCHOOL;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_diseases());
  for (int i = 0; i < 20; i++) {
    students_with_age[i] = 0;
    classrooms[i].clear();
  }
  total_classrooms = 0;
}


void School::prepare() {
  int diseases = population->get_diseases();
  for (int s = 0; s < diseases; s++) {
    susceptibles[s].clear();
    infectious[s].clear();
    Sympt[s] = S[s] = I[s] = 0;
    total_cases[s] = total_deaths[s] = 0;
  }
  for (int i = 0; i < 20; i++) {
    next_classroom[i] = 0;
    next_classroom_without_teacher[i] = 0;
  }
  close_date = INT_MAX;
  open_date = 0;
  if (Verbose > 2) {
    printf("prepare place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}


void School::get_parameters(int diseases) {
  char param_str[80];
  
  if (school_parameters_set) return;
  
  school_contacts_per_day = new double [ diseases ];
  school_contact_prob = new double** [ diseases ];
  
  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "school_contacts[%d]", s);
    get_param((char *) param_str, &school_contacts_per_day[s]);
    
    sprintf(param_str, "school_prob[%d]", s);
    n = get_param_matrix(param_str, &school_contact_prob[s]);
    if (Verbose > 1) {
      printf("\nschool_contact_prob:\n");
      for (int i  = 0; i < n; i++)  {
        for (int j  = 0; j < n; j++) {
          printf("%f ", school_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  
  get_param((char *) "school_classroom_size", &school_classroom_size);
  get_param((char *) "school_closure_policy", school_closure_policy);
  get_param((char *) "school_closure_day", &school_closure_day);
  get_param((char *) "school_closure_threshold", &school_closure_threshold);
  get_param((char *) "school_closure_period", &school_closure_period);
  get_param((char *) "school_closure_delay", &school_closure_delay);
  get_param((char *) "school_summer_schedule", &school_summer_schedule);
  get_param((char *) "school_summer_start", school_summer_start);
  get_param((char *) "school_summer_end", school_summer_end);
 
  school_parameters_set = 1;
}

int School::get_group(int disease, Person * per) {
  int age = per->get_age();
  if (age <12) { return 0; }
  else if (age < 16) { return 1; }
  else if (age < ADULT_AGE) { return 2; }
  else return 3;
}

double School::get_transmission_prob(int disease, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(disease, i);
  int col = get_group(disease, s);
  double tr_pr = school_contact_prob[disease][row][col];
  return tr_pr;
}

bool School::should_be_open(int day, int disease) {
  
  if (school_summer_schedule) {
    char current_day[10];
    strcpy(current_day, Sim_Date->get_MMDD(day));
    if (Verbose > 1) {
      fprintf(Statusfp,"School %s on summer schedule, current day is %s\n", label, current_day);
    }
    if (Date::day_is_between_MMDD(current_day, school_summer_start, school_summer_end)) {
      if (Verbose > 1) {
	fprintf(Statusfp,"School %s closed for summer\n", label);
	fflush(Statusfp);
      }
      return false;
    }
    else {
      if (Verbose > 1) {
	fprintf(Statusfp,"School %s is open\n", label);
	fflush(Statusfp);
      }
    }
  }

  if (strcmp(school_closure_policy, "global") == 0) {
    //
    // Setting school_closure_day > -1 overrides other global triggers
    //
    // close schools if the closure date has arrived (after a delay)
    if (school_closure_day > -1 && school_closure_day == day) {
      close_date = day+school_closure_delay;
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
    
    // Close schools if the global attack rate has reached the threshold
    // (with a delay)
    Disease * str = Pop.get_disease(disease);
    if (str->get_attack_rate() > school_closure_threshold) {
      if (is_open(day))
        close_date = day+school_closure_delay;
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
  }
  
  if (strcmp(school_closure_policy, "individual") == 0) {
    if (N == 0) return false;
    double frac = (double) Sympt[disease] / (double) N;
    if (frac >= school_closure_threshold) {
      if (is_open(day)) {
        close_date = day+school_closure_delay;
      }
      open_date = day+school_closure_delay+school_closure_period;
      return is_open(day);
    }
  }
  
  // if school_closure_policy is not recognized, then open
  return true;
}

double School::get_contacts_per_day(int disease) {
  return school_contacts_per_day[disease];
}

void School::enroll(Person * per) {
  N++;
  int age = per->get_age();
  if (age < ADULT_AGE) {
    students_with_age[age]++;
  }
  // else {students_with_age[19]++;}
}

void School::print(int disease) {
  fprintf(Statusfp, "Place %d label %s type %c ", id, label, type);
  fprintf(Statusfp, "S %d I %d N %d\n", S[disease], I[disease], N);
  for (int g = 0; g < 20; g++) {
    fprintf(Statusfp, "%d students with age %d | ",students_with_age[g],g);
  }
  fprintf(Statusfp, "\n");
  fflush(Statusfp);
}

void School::setup_classrooms() {
  for (int a = 0; a < 20; a++) {
    int n = students_with_age[a];
    next_classroom[a] = 0;
    next_classroom_without_teacher[a] = 0;
    if (n == 0) continue;
    int rooms = n / school_classroom_size; 
    if (n % school_classroom_size) rooms++;
    if (Verbose > 1) {
      fprintf(Statusfp, "school %d %s age %d number %d rooms %d\n",
	      id, label,a,n,rooms);
      fflush(Statusfp);
    }
    for (int c = 0; c < rooms; c++) {
      int new_id = Places.get_max_id() + 1;
      char new_label[128];
      sprintf(new_label, "%s-%02d-%02d", this->get_label(), a, c+1);
      new_label[0] = 'C';
      Place *p = new (nothrow) Classroom(new_id, new_label,
					 this->get_longitude(),
					 this->get_latitude(),
					 this,
					 this->get_population());
      Places.add_place(p);
      classrooms[a].push_back(p);
      total_classrooms++;
      if (Verbose > 1) {
	fprintf(Statusfp, "school %d %s age %d added classroom %d %s %d\n",
		id, label,a,c,p->get_label(),p->get_id());
	fflush(Statusfp);
      }
    }
  }
}


Place * School::assign_classroom(Person *per) {
  int age = per->get_age();
  if (age < ADULT_AGE) {
    // assign classroom to a student
    if (Verbose > 1) {
      fprintf(Statusfp,
	      "assign classroom for student %d in school %d %s for age %d == ",
	      per->get_id(), id, label, age);
      fflush(Statusfp);
    }
    assert(classrooms[age].size() > 0);

    // pick next classroom, round-robin
    int c = next_classroom[age];
    next_classroom[age]++;
    if (next_classroom[age]+1 > (int) classrooms[age].size())
      next_classroom[age] = 0;
    
    if (Verbose > 1) {
      fprintf(Statusfp, "room = %d %s %d\n",
	      c, classrooms[age][c]->get_label(), classrooms[age][c]->get_id());
      fflush(Statusfp);
    }
    return classrooms[age][c];
  }
  else {
    // assign classroom to a teacher
    if (Verbose > 1) {
      fprintf(Statusfp,
	      "assign classroom for teacher %d in school %d %s for age %d == ",
	      per->get_id(), id, label, age);
      fflush(Statusfp);
    }

    // pick next classroom, round-robin
    for (int a = 0; a < 20; a++) {
      int n = (int) classrooms[a].size();
      if (n == 0) continue;
      if (next_classroom_without_teacher[a] < n) {
	int c = next_classroom_without_teacher[a];
	next_classroom_without_teacher[a]++;
	if (Verbose > 1) {
	  fprintf(Statusfp, "room = %d %s %d\n",
		  c, classrooms[a][c]->get_label(), classrooms[a][c]->get_id());
	  fflush(Statusfp);
	}
	return classrooms[a][c];
      }
    }

    // all classrooms have a teacher, so assign to a random classroom
    int x = IRAND(0,total_classrooms-1);
    for (int a = 0; a < 20; a++) {
      for (unsigned int c = 0 ; c < classrooms[a].size(); c++) {
	if (x == 0) {
	  if (Verbose > 1) {
	    fprintf(Statusfp, "room = %d %s %d\n",
		    c, classrooms[a][c]->get_label(), classrooms[a][c]->get_id());
	    fflush(Statusfp);
	  }
	  return classrooms[a][c];
	}
	else { x--; }
      } 
    }
    printf("Help! Can't find classroom for teacher\n\n");
    abort();
  }
}


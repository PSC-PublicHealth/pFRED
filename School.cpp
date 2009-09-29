//
//
// File: School.cpp
//

#include "School.hpp"

int School_contacts_per_day;
double School_contact_prob;
char School_closure_policy[80];
int School_closure_day;
double School_closure_threshold;
int School_closure_period;
int School_closure_delay;
int School_parameters_set = 0;


School::School(int loc, char *lab, double lon, double lat, int diseases) {
  id = loc;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;
  type = SCHOOL;

  get_parameters();

  for (int d = 0; d < diseases; d++) {
    contacts_per_day[d] = School_contacts_per_day;
    contact_prob[d] = School_contact_prob;
  }
  reset();
}

void School::get_parameters() {
  if (School_parameters_set) return;
  get_param((char *) "school_contacts", &School_contacts_per_day);
  get_param((char *) "school_prob", &School_contact_prob);
  get_param((char *) "school_closure_policy", School_closure_policy);
  get_param((char *) "school_closure_day", &School_closure_day);
  get_param((char *) "school_closure_threshold", &School_closure_threshold);
  get_param((char *) "school_closure_period", &School_closure_period);
  get_param((char *) "school_closure_delay", &School_closure_delay);
  School_parameters_set = 1;
}

int School::should_be_open(int day, int dis) {
  extern double * Attack_rate;

  if (strcmp(School_closure_policy, "global") == 0) {
    //
    // Setting school_closure_day > -1 overrides other global triggers
    //
    // close schools if the closure date has arrived (after a delay)
    if (School_closure_day > -1 && School_closure_day == day) {
      close_date = day+School_closure_delay;
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }

    // Close schools if the global attack rate has reached the threshold
    // (with a delay)
    if (Attack_rate[dis] > School_closure_threshold) {
      if (is_open(day))
	close_date = day+School_closure_delay;
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }
  }

  if (strcmp(School_closure_policy, "reactive") == 0) {
    if (N == 0) return 0;
    double frac = (double) Sympt[dis] / (double) N;
    if (frac >= School_closure_threshold) {
      if (is_open(day)) {
	close_date = day+School_closure_delay;
      }
      open_date = day+School_closure_delay+School_closure_period;
      return is_open(day);
    }
  }

  // if school_closure_policy is not recognized, then open
  return 1;
}


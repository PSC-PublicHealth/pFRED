//
//
// File: Community.cpp
//

#include "Community.hpp"

int Community_contacts_per_day;
double Community_contact_prob;
char Community_closure_policy[80];
int Community_closure_day;
double Community_closure_threshold;
int Community_closure_period;
int Community_closure_delay;
int Community_parameters_set = 0;


Community::Community(int loc, char *lab, double lon, double lat, int diseases) {
  // printf("Community setup entered\n"); fflush(stdout);
  id = loc;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;
  type = COMMUNITY;

  get_parameters();

  for (int d = 0; d < diseases; d++) {
    contacts_per_day[d] = Community_contacts_per_day;
    contact_prob[d] = Community_contact_prob;
  }
  reset();
}

void Community::get_parameters() {
  if (Community_parameters_set) return;
  get_param((char *) "community_contacts", &Community_contacts_per_day);
  get_param((char *) "community_prob", &Community_contact_prob);
  // get_param((char *) "community_closure_policy", Community_closure_policy);
  // get_param((char *) "community_closure_day", &Community_closure_day);
  // get_param((char *) "community_closure_threshold", &Community_closure_threshold);
  // get_param((char *) "community_closure_period", &Community_closure_period);
  // get_param((char *) "community_closure_delay", &Community_closure_delay);
  Community_parameters_set = 1;
}

int Community::should_be_open(int day, int dis) {

  // if community_closure_policy is not recognized, then open
  return 1;

}


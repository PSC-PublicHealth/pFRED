//
//
// File: Workplace.cpp
//

#include "Workplace.hpp"

int Workplace_contacts_per_day;
double Workplace_contact_prob;
char Workplace_closure_policy[80];
int Workplace_closure_day;
double Workplace_closure_threshold;
int Workplace_closure_period;
int Workplace_closure_delay;
int Workplace_parameters_set = 0;


Workplace::Workplace(int loc, char *lab, double lon, double lat, int diseases) {
  id = loc;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;
  type = WORKPLACE;

  get_parameters();

  for (int d = 0; d < diseases; d++) {
    contacts_per_day[d] = Workplace_contacts_per_day;
    contact_prob[d] = Workplace_contact_prob;
  }
  reset();
}

void Workplace::get_parameters() {
  if (Workplace_parameters_set) return;
  get_param((char *) "workplace_contacts", &Workplace_contacts_per_day);
  get_param((char *) "workplace_prob", &Workplace_contact_prob);
  // get_param((char *) "workplace_closure_policy", Workplace_closure_policy);
  // get_param((char *) "workplace_closure_day", &Workplace_closure_day);
  // get_param((char *) "workplace_closure_threshold", &Workplace_closure_threshold);
  // get_param((char *) "workplace_closure_period", &Workplace_closure_period);
  // get_param((char *) "workplace_closure_delay", &Workplace_closure_delay);
  Workplace_parameters_set = 1;
}

int Workplace::should_be_open(int day, int dis) {

  // if workplace_closure_policy is not recognized, then stay open
  return 1;

}


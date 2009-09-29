//
//
// File: Household.cpp
//

#include "Household.hpp"

int Household_contacts_per_day;
double Household_contact_prob;
int Household_parameters_set = 0;


Household::Household(int loc, char *lab, double lon, double lat, int diseases) {
  id = loc;
  strcpy(label,lab);
  longitude = lon;
  latitude = lat;
  type = HOUSEHOLD;

  get_parameters();

  for (int d = 0; d < diseases; d++) {
    contacts_per_day[d] = Household_contacts_per_day;
    contact_prob[d] = Household_contact_prob;
  }
  reset();
}

void Household::get_parameters() {
  if (Household_parameters_set) return;
  get_param((char *) "household_contacts", &Household_contacts_per_day);
  get_param((char *) "household_prob", &Household_contact_prob);
  // get_param((char *) "household_closure_policy", Household_closure_policy);
  // get_param((char *) "household_closure_day", &Household_closure_day);
  // get_param((char *) "household_closure_threshold", &Household_closure_threshold);
  // get_param((char *) "household_closure_period", &Household_closure_period);
  // get_param((char *) "household_closure_delay", &Household_closure_delay);
  Household_parameters_set = 1;
}



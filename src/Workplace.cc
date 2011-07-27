/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Workplace.cc
//

#include "Workplace.h"
#include "Global.h"
#include "Params.h"
#include "Random.h"
#include "Person.h"
#include "Disease.h"
#include "Place_List.h"
#include "Office.h"

//Private static variables that will be set by parameter lookups
double * Workplace::Workplace_contacts_per_day;
double *** Workplace::Workplace_contact_prob;
int Workplace::office_size = 50;

//Private static variable to assure we only lookup parameters once
bool Workplace::Workplace_parameters_set = false;

Workplace::Workplace(int loc, const char *lab, double lon, double lat, Place *container, Population *pop) {
  type = WORKPLACE;
  setup(loc, lab, lon, lat, container, pop);
  get_parameters(population->get_diseases());
  offices.clear();
  next_office = 0;
}

void Workplace::get_parameters(int diseases) {
  char param_str[80];
  
  if (Workplace::Workplace_parameters_set) return;
  
  Workplace::Workplace_contacts_per_day = new double [ diseases ];
  Workplace::Workplace_contact_prob = new double** [ diseases ];
  
  // people per office
  get_param((char *) "office_size", &Workplace::office_size);

  for (int s = 0; s < diseases; s++) {
    int n;
    sprintf(param_str, "workplace_contacts[%d]", s);
    get_param((char *) param_str, &Workplace::Workplace_contacts_per_day[s]);
    
    sprintf(param_str, "workplace_prob[%d]", s);
    n = get_param_matrix(param_str, &Workplace::Workplace_contact_prob[s]);
    if (Global::Verbose > 1) {
      printf("\nWorkplace_contact_prob:\n");
      for (int i  = 0; i < n; i++) {
        for (int j  = 0; j < n; j++) {
          printf("%f ", Workplace::Workplace_contact_prob[s][i][j]);
        }
        printf("\n");
      }
    }
  }
  
  Workplace::Workplace_parameters_set = true;
}

void Workplace::prepare() {
  for (int s = 0; s < diseases; s++) {
    susceptibles[s].reserve(N);
    infectious[s].reserve(N);
    total_cases[s] = total_deaths[s] = 0;
  }
  update(0);
  open_date = 0;
  close_date = INT_MAX;
  next_office = 0;
  if (Global::Verbose > 2) {
    printf("prepare place: %d\n", id);
    print(0);
    fflush(stdout);
  }
}

int Workplace::get_group(int disease, Person * per) {
  return 0;
}

double Workplace::get_transmission_prob(int disease, Person * i, Person * s) {
  // i = infected agent
  // s = susceptible agent
  int row = get_group(disease, i);
  int col = get_group(disease, s);
  double tr_pr = Workplace::Workplace_contact_prob[disease][row][col];
  return tr_pr;
}

double Workplace::get_contacts_per_day(int disease) {
  return Workplace::Workplace_contacts_per_day[disease];
}

void Workplace::setup_offices() {
  int rooms = N / Workplace::office_size;
  next_office = 0;
  if (N % Workplace::office_size) rooms++;
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp,
	    "workplace %d %s number %d rooms %d\n", id, label,N,rooms);
    fflush(Global::Statusfp);
  }
  for (int i = 0; i < rooms; i++) {
    int new_id = Global::Places.get_max_id() + 1;
    char new_label[128];
    sprintf(new_label, "%s-%03d", this->get_label(), i);
    Place *p = new Office(new_id, new_label,
			  this->get_longitude(),
			  this->get_latitude(),
			  this,
			  this->get_population());
    Global::Places.add_place(p);
    offices.push_back(p);
    if (Global::Verbose>1) {
      fprintf(Global::Statusfp,
	      "workplace %d %s added office %d %s %d\n",
	      id, label,i,p->get_label(),p->get_id());
      fflush(Global::Statusfp);
    }
  }
}

Place * Workplace::assign_office(Person *per) {
  if (Global::Verbose>1) {
    fprintf(Global::Statusfp,
	    "assign office for person %d at workplace %d %s == ",
	    per->get_id(), id, label);
    fflush(Global::Statusfp);
  }

  // pick next office, round-robin
  int i = next_office;

  if (Global::Verbose>1) {
    fprintf(Global::Statusfp,
	    "office = %d %s %d\n",
	    i, offices[i]->get_label(), offices[i]->get_id());
    fflush(Global::Statusfp);
  }

  next_office++;
  if (next_office+1 > (int) offices.size())
    next_office = 0;
  return offices[i];
}

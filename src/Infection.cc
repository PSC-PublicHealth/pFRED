/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

// Infection.cc
// ------------
// Determines the transition dates for this person.

#include <stdexcept>
#include "limits.h"

#include "Infection.h"
#include "Evolution.h"
#include "Transmission.h"
#include "Global.h"
#include "Person.h"
#include "Place.h"
#include "Random.h"
#include "Disease.h"
#include "Health.h"
#include "IntraHost.h"
#include <map>
#include <vector>
#include <float.h>

using std::out_of_range;

Infection::Infection(Disease *disease, Person* infector, Person* host, Place* place, int day) {

  // general
  this->disease = disease;
  this->id = disease->get_id();
  this->infector = infector;
  this->host = host;
  this->place = place;
  this->trajectory = NULL;
  infectee_count = 0;

  //  trajectory_infectivity_threshold = IntraHost::singletonIntraHost().get_infectivity_threshold(this);
  //  trajectory_symptomaticity_threshold = IntraHost::singletonIntraHost().get_symptomaticity_threshold(this);

  isSusceptible = true;
  trajectory_infectivity_threshold = 0;
  trajectory_symptomaticity_threshold = 0;

  // parameters
  infectivity_multp = 1.0;
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;

  // chrono
  exposure_date = day;
  recovery_period = disease->get_days_recovered();

  offset = 0;

  if (symptomatic_date != -1) {
    will_be_symptomatic = true;
    }
  else {
    will_be_symptomatic = false;
    }

  report_infection(day);
  host->set_changed();
  }

void Infection::determine_transition_dates() {
  // returns the first date that the agent changes state
  bool was_latent = true;
  bool was_incubating = true;

  latent_period = 0;
  asymptomatic_period = 0;
  symptomatic_period = 0;
  incubation_period = 0;

  infectious_date = -1;
  symptomatic_date = -1;
  asymptomatic_date = -1;
  recovery_date = exposure_date + trajectory->get_duration();

  Trajectory::iterator trj_it = Trajectory::iterator(trajectory);

  while (trj_it.has_next()) {
    bool infective = (trj_it.next().infectivity > trajectory_infectivity_threshold);
    bool symptomatic = (trj_it.next().symptomaticity > trajectory_symptomaticity_threshold);
    bool asymptomatic = (infective && !(symptomatic));

    if (infective && was_latent) {
      latent_period = trj_it.get_current();
      infectious_date = exposure_date + latent_period; // TODO

      if (asymptomatic & was_latent) {
        asymptomatic_date = infectious_date;
        }

      was_latent = false;
      }

    if (symptomatic && was_incubating) {
      incubation_period = trj_it.get_current();
      symptomatic_date = exposure_date + incubation_period;
      was_incubating = false;
      }

    if (symptomatic) {
      symptomatic_period++;
      }

    if (asymptomatic) {
      asymptomatic_period++;
      }
    }
  }

void Infection::become_infectious() {
  disease->decrement_E_count();

  if (is_symptomatic()) {
    status = 'I';
    disease->increment_I_count();
    disease->increment_c_count();
    }
  else {
    status = 'i';
    disease->increment_i_count();
    }

  host->set_changed();
  }

void Infection::become_susceptible() {
  status = 'S';
  disease->decrement_r_count();
  }

void Infection::become_unsusceptible() {
  isSusceptible = false;
  disease->decrement_S_count();
  host->set_changed();
  }

void Infection::become_symptomatic() {
  if (status == 'i') {
    disease->decrement_i_count();
    }

  if (status != 'I') {
    status = 'I';
    disease->increment_I_count();
    }

  host->set_changed();
  }

void Infection::recover() {
  if (status == 'I') {
    disease->decrement_I_count();
    }
  else if (status == 'i') {
    disease->decrement_i_count();
    }

  status = 'R';

  if (recovery_period > -1) {
    disease->increment_r_count();
    }
  else {
    disease->increment_R_count();
    }

  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  host->set_changed(); // note that the infection state changed
  }

void Infection::remove() {
  if (status == 'R') {
    return;
    }

  if (status == 'E') {
    disease->decrement_E_count();
    }

  if (status == 'I') {
    disease->decrement_I_count();
    }

  if (status == 'i') {
    disease->decrement_i_count();
    }

  status = 'R';
  host->set_changed(); // note that the infection state changed
  infectivity = 0.0;
  susceptibility = 0.0;
  symptoms = 0.0;
  }
void Infection::update(int today) {
  if(trajectory == NULL) return;

  // offset used for dummy infections only
  int days_post_exposure = today - exposure_date + offset;

  Trajectory::point trajectory_point = trajectory->get_data_point(days_post_exposure);
  infectivity = trajectory_point.infectivity;
  symptoms = trajectory_point.symptomaticity;

  if (today == get_infectious_date()) {
    host->become_infectious(disease);
    }

  if (today == get_symptomatic_date()) {
    host->become_symptomatic(disease);
    }

  if (today == get_recovery_date()) {
    host->recover(disease);
    }

  if (today == get_susceptible_date()) {
    host->become_susceptible(disease->get_id());
    }

  if (today == get_unsusceptible_date()) {
    host->become_unsusceptible(disease->get_id());
    }

  if(today != get_recovery_date()) {
    vector<int> strains;
    trajectory->getAllStrains(strains);
    host->addPrevalence(disease->get_id(), strains);
    }
  }

void Infection::modify_symptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");

  // after symptomatic period
  if (today >= get_recovery_date())
    throw out_of_range("cannot modify: past symptomatic period");

  // before symptomatic period
  else if (today < get_symptomatic_date()) {
    trajectory->modifySympPeriod(symptomatic_date, symptomatic_period * multp);
    determine_transition_dates();
    }

  // during symptomatic period
  // if days_left becomes 0, we make it 1 so that update() sees the new dates
  else {
    //int days_into = today - get_symptomatic_date();
    int days_left = get_recovery_date() - today;
    days_left *= multp;
    trajectory->modifySympPeriod(today - exposure_date + offset, days_left);
    determine_transition_dates();
    }

  host->set_changed();
  }

void Infection::modify_asymptomatic_period(double multp, int today) {
  // negative multiplier
  if (multp < 0)
    throw out_of_range("cannot modify: negative multiplier");

  // after asymptomatic period
  if (today >= get_symptomatic_date()) {
    printf("ERROR: Person %d %d %d\n", host->get_id(), today, get_symptomatic_date());
    throw out_of_range("cannot modify: past asymptomatic period");
    }

  // before asymptomatic period
  else if (today < get_infectious_date()) {
    trajectory->modifyAsympPeriod(exposure_date, asymptomatic_period * multp, get_symptomatic_date());
    determine_transition_dates();
    }

  // during asymptomatic period
  else {
    //int days_into = today - get_infectious_date();
    int days_left = get_symptomatic_date() - today;
    trajectory->modifyAsympPeriod(today - exposure_date, days_left * multp, get_symptomatic_date());
    determine_transition_dates();
    }

  host->set_changed();
  }

void Infection::modify_infectious_period(double multp, int today) {
  if (today < get_symptomatic_date())
    modify_asymptomatic_period(multp, today);

  modify_symptomatic_period(multp, today);
  }

void Infection::modify_develops_symptoms(bool symptoms, int today) {
  if ((today >= get_symptomatic_date() && get_asymptomatic_date() == -1) || (today >= get_recovery_date()) )
    throw out_of_range("cannot modify: past symptomatic period");

  if (will_be_symptomatic != symptoms) {
    symptomatic_period = will_be_symptomatic ? disease->get_days_symp() : 0;
    trajectory->modifyDevelopsSymp(get_symptomatic_date(), symptomatic_period);
    determine_transition_dates();
    host->set_changed();
    }
  }

void Infection::print() const {
  printf("Infection of disease type: %i in person %i current status: %c\n"
         "periods:  latent %i, asymp: %i, symp: %i recovery: %i \n"
         "dates: exposed: %i, infectious: %i, symptomatic: %i, recovered: %i susceptible: %i\n"
         "will have symp? %i, suscept: %.3f infectivity: %.3f "
         "infectivity_multp: %.3f symptms: %.3f\n",
         disease->get_id(),
         host->get_id(),
         status,
         latent_period,
         asymptomatic_period,
         symptomatic_period,
         recovery_period,
         exposure_date,
         get_infectious_date(),
         get_symptomatic_date(),
         get_recovery_date(),
         get_susceptible_date(),
         will_be_symptomatic,
         susceptibility,
         infectivity,
         infectivity_multp,
         symptoms);
  }

// static
Infection *Infection::get_dummy_infection(Disease *s, Person* host, int day) {
  Infection* i = new Infection(s, NULL, host, NULL, day);
  i->dummy = true;
  i->offset = i->infectious_date - day;
  return i;
  }

void Infection::transmit(Person *infectee, Transmission *transmission) {
  int day = transmission->get_exposure_date() - exposure_date + offset;
  map<int, double> *loads = trajectory->getInoculum(day);
  transmission->setInitialLoads(loads);
  infectee->getInfected(this->disease, transmission);
  }

void Infection::addTransmission(Transmission *transmission) {
  transmissions.push_back(transmission);

  int day = transmission->get_exposure_date() - exposure_date + offset;

  map<int, double> *loads = transmission->getInitialLoads();

  if(trajectory != NULL) { // this person is already infected by current disease
    map<int, double> *currentLoads = trajectory->getCurrentLoads(day);

    // Ignore further transmissions of the same strain
    for( map<int, double>::iterator it = currentLoads->begin(); it != currentLoads->end(); it++ ) {
      loads->insert( pair<int, double> (it->first, it->second) );
      }
    }

  disease->get_evolution()->doEvolution(this, loads);

  // TODO update in case infectious date etc is today
  // update(exposure_date);

  if(infectious_date == -1) {
    trajectory = NULL;

    if(Verbose > 1) {
      printf("New transmission failed for person %d. \n", host->get_id());
      }
    }
  else {
    if(Verbose > 1) {
      printf("Transmission succeeded for person %d.\n", host->get_id());
      printf("New trajectories:\n");
      trajectory->print();
      printf("Dates: %d, %d, %d, %d, %d \n", exposure_date, infectious_date,  symptomatic_date, asymptomatic_date, recovery_date);
      }

    status = 'E';
    disease->increment_E_count();
    disease->increment_C_count();
    disease->insert_into_infected_list(host);

    vector<int> strains;
    trajectory->getAllStrains(strains);
    host->addIncidence(disease->get_id(), strains);
    host->addPrevalence(disease->get_id(), strains);

    if(isSusceptible && susceptibility_period == 0) {
      host->become_unsusceptible(disease->get_id());
      }
    }
  }

void Infection::setTrajectory(Trajectory *trajectory) {
  this->trajectory = trajectory;
  this->determine_transition_dates();
  }

void Infection::report_infection(int day) const {
  if (Infectionfp == NULL) return;

  fprintf(Infectionfp, "day %d dis %d host %d age %.3f "
          " from %d inf_age %.3f at %c ",
          day, id,
          host->get_id(),
          host->get_real_age(day),
          infector == NULL ? -1 : infector->get_id(),
          infector == NULL ? -1 : infector->get_real_age(day),
          place == NULL ? 'X' : place->get_type());

  if (Track_infection_events > 1)
    fprintf(Infectionfp,
            "| PERIODS  latent %d asymp %d symp %d recovery %d ",
            latent_period,
            asymptomatic_period,
            symptomatic_period,
            recovery_period);

  if (Track_infection_events > 2)
    fprintf(Infectionfp,
            "| DATES exp %d inf %d symp %d rec %d sus %d ",
            exposure_date,
            get_infectious_date(),
            get_symptomatic_date(),
            get_recovery_date(),
            get_susceptible_date());

  if (Track_infection_events > 3)
    fprintf(Infectionfp,
            "| will_be_symp? %d susc %.3f infect %.3f "
            "inf_multp %.3f symptms %.3f ",
            will_be_symptomatic,
            susceptibility,
            infectivity,
            infectivity_multp,
            symptoms);

  fprintf(Infectionfp, "\n");
  fflush(Infectionfp);
}


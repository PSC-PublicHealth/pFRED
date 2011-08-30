/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior.h
//

#ifndef _FRED_BEHAVIOR_H
#define _FRED_BEHAVIOR_H

#include <vector>
#include "Global.h"
#include "Behavior_Model.h"

class Person;
class Disease;
class Transmission;

extern int V_count;

class Behavior {

public:

  /**
   * Constructor
   * @param p a pointer to the agent who will exhibit this behavior
   */
  Behavior(Person *p);
  ~Behavior() {}

  /**
    * Perform the daily update for this object
    *
    * @param day the simulation day
    */
  void update(int day) {
    model->update(day);
    V_count += will_accept_vaccine(0);
  }

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept vaccine, <code>false</code> if not
   * @see Behavior_Model::will_accept_vaccine(int disease)
   */
  bool will_accept_vaccine(int disease) {
    return model->will_accept_vaccine(disease);
  }

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept another vaccine dose, <code>false</code> if not
   * @see Behavior_Model::will_accept_another_vaccine_dose(int disease)
   */
  bool will_accept_another_vaccine_dose(int disease) {
    return model->will_accept_another_vaccine_dose(disease);
  }

  /**
   *
   * @return <code>true</code> if agent will keep kids home, <code>false</code> otherwise
   */
  bool will_keep_kids_home() {
    return false;
  }


  /**
   * This method simply checks to see if agent will accept vaccine for disease = 0
   * @return <code>true</code> if agent will accept vaccine, <code>false</code> if not
   */
  bool acceptance_of_vaccine() {
    return will_accept_vaccine(0);
  }

  /**
   * This method simply checks to see if agent will accept another vaccine dose for disease = 0
   *
   * @return <code>true</code> if agent will accept another vaccine dose, <code>false</code> if not
   */
  bool acceptance_of_another_vaccine_dose() {
    return will_accept_another_vaccine_dose(0);
  }

  /**
   * Does nothing right now
   * @param disease pointer to a Disease object
   * @param transmission pointer to a Transmission object
   */
  void getInfected(Disease *disease, Transmission *transmission) {};

  bool is_staying_home(int day) { return model->is_staying_home(day); }

private:
  void get_parameters();
  Behavior_Model* model;       // behavior model does all the real work
};

#endif // _FRED_BEHAVIOR_H


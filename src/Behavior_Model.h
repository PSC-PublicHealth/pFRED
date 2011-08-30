/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Behavior_Model.h
//

#ifndef _FRED_COGNITIVE_MODEL_H
#define _FRED_COGNITIVE_MODEL_H

class Behavior_Model {
 public:
  /**
   * Default constructor
   */
  Behavior_Model() {}
  ~Behavior_Model() {}

  /**
    * Perform the daily update for this object
    *
    * @param day the simulation day
    */
  virtual void update(int day) = 0;

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept vaccine, <code>false</code> if not
   */
  virtual bool will_accept_vaccine(int disease) = 0;

  /**
   * @param disease which disease
   * @return <code>true</code> if agent will accept another vaccine dose, <code>false</code> if not
   */
  virtual bool will_accept_another_vaccine_dose(int disease) = 0;

  virtual bool is_staying_home(int day) = 0;
};

#endif // _FRED_COGNITIVE_MODEL_H


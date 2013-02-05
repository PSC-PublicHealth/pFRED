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
// File: Attitude.h
//

#ifndef _FRED_ATTITUDE_H
#define _FRED_ATTITUDE_H

#include "Global.h"
#include "Random.h"
#include "Behavior.h"

class Attitude {
public:
  /**
   * Default constructor
   */
  Attitude(int _index);

  ~Attitude() {}

  /**
    * Perform the daily update for this object
    *
    * @param day the simulation day
    */
  void update(int day);

  // access functions
  void set_strategy(int strat) { strategy = strat; }
  void set_probability(double prob) { probability = prob; }
  void set_frequency(int freq) { frequency = freq; }
  void set_willing(bool decision) { willing = decision; }
  int get_strategy() { return strategy; }
  double get_probability () { return probability; }
  double get_frequency () { return frequency; }
  bool is_willing() { return willing; }

 private:
  int index;
  int strategy;
  double probability;
  int frequency;
  int expiration;
  bool willing;
  Behavior_params * params;

};

#endif // _FRED_ATTITUDE_H


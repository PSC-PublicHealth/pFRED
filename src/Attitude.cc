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
// File: Attitude.cc
//

#include "Attitude.h"
#include "Global.h"
#include "Random.h"
#include "Utils.h"
#include "Behavior.h"

Attitude::Attitude(int _index) {
  index = _index;
  expiration = 0;
  params = Behavior::get_behavior_params(index);
  
  // pick a strategy for this individual based on the population market shares
  strategy = draw_from_distribution(params->strategy_cdf_size, params->strategy_cdf);

  // set the other attitude parameters based on the strategy
  switch (strategy) {

  case REFUSE:
    willing = false;
    probability = 0.0;
    frequency = 0;
    break;

  case ACCEPT:
    willing = true;
    probability = 1.0;
    frequency = 0;
    break;

  default: // REFUSE
    willing = false;
    probability = 0.0;
    frequency = 0;
    break;
  }

  FRED_VERBOSE(1,
	       "created ATTITUDE %d name %d strategy %d freq %d expir %d probability %f\n",
	       index, params->name, strategy,
	       frequency, expiration, probability);
}

void Attitude::update(int day) {
  
  FRED_VERBOSE(1,
	       "update ATTITUDE %d name %s day %d strategy %d freq %d expir %d probability %f\n",
	       index, params->name, day, strategy,
	       frequency, expiration, probability);

  if (frequency > 0 && expiration <= day) {
    double r = RANDOM();
    willing = (r < probability);
    expiration = day + frequency;
  }

  FRED_VERBOSE(1,
	       "update ATTITUDE %d willing %d  %d expir %d\n",
	       index, willing?1:0, expiration);

}



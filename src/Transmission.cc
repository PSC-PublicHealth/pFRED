/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
// File: Transmission.h
//

#include "Transmission.h"

Transmission :: ~Transmission() {
  delete initialLoads;
}

void Tranmsission :: print()
{
  printf("Infector: %i, place: %i, date exposed: %i", infector->get_id(), place->get_id(), exposure_date);
}



//
//
// File: SynDem.h
//

#ifndef _SYNDEM_H
#define _SYNDEM_H

#include "Disease.hpp"
#include "Pop.hpp"
#include "Loc.hpp"
#include "Params.hpp"

void get_global_parameters();
void setup(char *paramfile);
void cleanup(int run);
void run_sim(int run);

#endif // _SYNDEM_H

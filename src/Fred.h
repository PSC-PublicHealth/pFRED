/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Fred.h
//

#ifndef _FRED_H

int main(int argc, char* argv[]);
void setup(char *paramfile);
void cleanup(int run);
void run_sim(int run);

//class Population;

//class Fred {
//
//public:
//  Fred();
//  virtual ~Fred();
//  void setup(char *paramfile);
//  void cleanup(int run);
//  void run_sim(int run);
//
//private:
//  Population static_pop;
//
//};

#define _FRED_H

#endif // _FRED_H

/*
  Copyright 2009 by the University of Pittsburgh
  Licensed under the Academic Free License version 3.0
  See the file "LICENSE" for more information
*/

//
//
// File: Utils.h
//

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <stdarg.h>

using namespace std;

namespace Utils{
	void fred_abort(const char* format,...);
	//void fred_warning(const char* format,...);
	void fred_end();
}
#endif /* UTILS_H_ */

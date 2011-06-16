/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: Catchment.h
//


#ifndef CATCHMENT_H_
#define CATCHMENT_H_

#include <stdlib>
#include <iostream>
#include <string>

#include "Global.h"
#include "Utils.h"

class Catchment {
public:
	Catchment(void);
	~Catchment(void);

	void fill_with_zipcodes(string zip_code);
	void fill_with_latlong_box(double lat_0, double lon_0, double lat_1, double lon_1);
	void fill_with_FIPS(string FIPSCode);
	void fill_with circle(double lat_0, double lon_0, double radius);





private:
	vector <Person *> pop;
	vector <Household *> households; // May not be working yet.
};

#endif /* CATCHMENT_H_ */

/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */

//
//
// File: Perceptions.h
//

#ifndef _FRED_PERCEPTIONS_H
#define _FRED_PERCEPTIONS_H

class Person;

class Perceptions {
public:
	Perceptions(Person *p);
	virtual void reset();
	virtual void update(int day);
	virtual int will_keep_kids_home() { return keep_kids_home; }
	// double get_perceived_susceptibility() { return perceived_susceptibility; }
	// double get_perceived_severity() { return perceived_severity; }
	// double get_perceived_benefits() { return perceived_benefits; }
	// double get_perceived_barriers() { return perceived_barriers; }
	// double get_self_efficacy() { return self_efficacy; }
	
private:
	void HBM(int day);
	Person * self;
	double perceived_susceptibility;
	double perceived_severity;
	double perceived_benefits;
	double perceived_barriers;
	double self_efficacy;
	int keep_kids_home;	
	
protected:
	Perceptions() { }
};

#endif // _FRED_PERCEPTIONS_H


/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: Utils.cc
//


#include <stdio.h>
#include <string>
#include <iostream>
#include <stdarg.h>


#include "Utils.h"
#include "Global.h"

using namespace std;

void Utils::fred_abort(const char* format, ...){

	if(ErrorLogfp != NULL){
		va_list ap;
		va_start(ap,format);
		fprintf(ErrorLogfp,"FRED ERROR: ");
		vfprintf(ErrorLogfp,format,ap);
		va_end(ap);
		fflush(ErrorLogfp);
	}
	va_list ap;
	va_start(ap,format);
	printf("FRED ERROR: ");
	vprintf(format,ap);
	va_end(ap);
	fflush(stdout);

	fred_end();
}

void Utils::fred_warning(const char* format, ...){

	if(ErrorLogfp != NULL){
		va_list ap;
		va_start(ap,format);
		fprintf(ErrorLogfp,"FRED WARNING: ");
		vfprintf(ErrorLogfp,format,ap);
		va_end(ap);
		fflush(ErrorLogfp);
	}
	va_list ap;
	va_start(ap,format);
	printf("FRED WARNING: ");
	vprintf(format,ap);
	va_end(ap);
	fflush(stdout);
}


void Utils::fred_end(void){
	// This is a function that cleans up FRED and exits
	if (Outfp != NULL) fclose(Outfp);
	if (Tracefp != NULL) fclose(Tracefp);
	if (Infectionfp != NULL) fclose(Infectionfp);
	if (VaccineTracefp != NULL) fclose(VaccineTracefp);
	if (Prevfp != NULL) fclose(Prevfp);
	if (Incfp != NULL) fclose(Incfp);
	abort();
}

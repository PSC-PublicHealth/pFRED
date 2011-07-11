/*
 Copyright 2009 by the University of Pittsburgh
 Licensed under the Academic Free License version 3.0
 See the file "LICENSE" for more information
 */
//
//
// File: Utils.cc
//

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <stdarg.h>

#include "Utils.h"
#include "Global.h"

using namespace std;

void Utils::fred_abort(const char* format, ...){

	if(Global::ErrorLogfp != NULL){
		va_list ap;
		va_start(ap,format);
		fprintf(Global::ErrorLogfp,"FRED ERROR: ");
		vfprintf(Global::ErrorLogfp,format,ap);
		va_end(ap);
		fflush(Global::ErrorLogfp);
	}
	va_list ap;
	va_start(ap,format);
	printf("FRED ERROR: ");
	vprintf(format,ap);
	va_end(ap);
	fflush(stdout);

	fred_end();
        abort();
}

void Utils::fred_warning(const char* format, ...){

	if(Global::ErrorLogfp != NULL){
		va_list ap;
		va_start(ap,format);
		fprintf(Global::ErrorLogfp,"FRED WARNING: ");
		vfprintf(Global::ErrorLogfp,format,ap);
		va_end(ap);
		fflush(Global::ErrorLogfp);
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
	if (Global::Outfp != NULL) fclose(Global::Outfp);
	if (Global::Tracefp != NULL) fclose(Global::Tracefp);
	if (Global::Infectionfp != NULL) fclose(Global::Infectionfp);
	if (Global::VaccineTracefp != NULL) fclose(Global::VaccineTracefp);
	if (Global::Prevfp != NULL) fclose(Global::Prevfp);
	if (Global::Incfp != NULL) fclose(Global::Incfp);
}

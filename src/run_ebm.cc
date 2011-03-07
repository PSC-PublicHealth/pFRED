// FILE: run_ebm.cc
// standalone driver for the ebm class

#include "AEB.h"
#include "EBM.h"
#include <string>

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

int main() {
	FILE *fpt;
	double max_f = 11000;
	double max_i = 1.000;
	double max_v = 100.0;
	double sigma = 0.001;
	double mu = 0.01;
/*	srand((unsigned)time(0));
	//experiment 1
	fpt = fopen("exp1.dat","w");

	fprintf(fpt,"IN,V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,F0,F1,F2,F3,F4,F5,F6,F7,F8,F9,I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,C0,C1,C2,C3,C4,C5,C6,C7,C8,C9\n");

	for (int run = 0; run < 10; run++) {
		double inoculum_particles = AEB::normal_draw(mu,sigma);
		if (inoculum_particles > 0.05 || inoculum_particles < 0.005) { mu = 0.01; continue; }
		EBM * ebm = new EBM( );
		ebm->set_inoculum_particles(inoculum_particles);
		ebm->solve_sode();
		double * v = ebm->get_viral_titer_data();
		double * f = ebm->get_interferon_data();
		double * i = ebm->get_infected_cells_data();
		int duration = ebm->get_duration();
		int random_day = rand() % ebm->get_duration();
		mu = ( ( v[random_day] / 100 ) * ( 0.05 - 0.005) ) + 0.005;
		fprintf(fpt,"%f,", inoculum_particles);
		for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f,",(v[d]/max_v));
		}// ^ viral titer
		for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f,",(f[d]/max_f));
		}// ^ interferon
		for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f,",(i[d]/max_i));
		}// ^ infected cells
		for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f",max(f[d]/max_f,i[d]/max_i));
			if (d != duration-1) { fprintf(fpt,","); }
			else {fprintf(fpt,"\n");}
		}// ^ compsite symptomaticity
		delete[] v;delete[] f;delete[] i;delete ebm;
	}
	fclose(fpt);
*/
	//experiment 2
	//fpt = fopen("exp2.dat","w");
	fpt = stdout;
	fprintf(fpt,"IN,V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,F0,F1,F2,F3,F4,F5,F6,F7,F8,F9,I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,C0,C1,C2,C3,C4,C5,C6,C7,C8,C9\n");
	double inoculum_particles = 0.05;
	double step = 0.0005;
	while (inoculum_particles <= 0.10) {
		EBM * ebm = new EBM(1);
    ebm->setup();
		//ebm->set_inoculum_particles(inoculum_particles);
		//ebm->solve_sode();
		
    ebm->set_V(0, inoculum_particles);

		double * v = ebm->get_viral_titer_data(0);
		double * f = ebm->get_interferon_data(0);
		double * i = ebm->get_infected_cells_data(0);

		int duration = ebm->get_duration();
		
    fprintf(fpt,"duration: %d \n,", duration);
    
    fprintf(fpt,"inoc_particles: %f, ", inoculum_particles);
    for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f, ",(v[d]/max_v));
		}// ^ viral titer

    for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f, ",(f[d]/max_f));
		}// ^ interferon

    for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f,",(i[d]/max_i));
		}// ^ infected cells

    for (int d=0; d<duration; d++) {
			fprintf(fpt,"%f",max(f[d]/max_f,i[d]/max_i));
			if (d != duration-1) { fprintf(fpt,","); }
			else {fprintf(fpt,"\n");}
		}// ^ compsite symptomaticity

    delete[] v;delete[] f;delete[] i;delete ebm;
		inoculum_particles += step;
    break;
	}
	fclose(fpt);

}	



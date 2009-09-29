##################### SynDEM src makefile ###########################

#################  Compiler Flags ##############################
CPP = g++
# CPPFLAGS = -O2 -Wall
CPPFLAGS = -g -Wall

###############################################

%.o:%.cpp %.hpp
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

ABM = Params.o Place.o Loc.o Person.o Pop.o Disease.o Random.o Household.o Community.o School.o Workplace.o

SYNDEM: Syndem.cpp Syndem.hpp $(ABM)
	$(CPP) $(CPPFLAGS) Syndem.cpp $(ABM) -o SYNDEM

print:
	enscript Person.cpp Place.cpp Disease.cpp Loc.cpp Pop.cpp Syndem.cpp Random.cpp
	enscript Person.hpp Place.hpp Disease.hpp Loc.hpp Pop.hpp Syndem.hpp Random.hpp

##############################################

clean:
	rm -f *.o *~ SYNDEM out* trace*

tar: clean
	cd ..
	tar cvf SYNDEM.tar SYNDEM

dist:
	make clean
	(cd ..; tar cvf SYNDEM-`date +"%Y-%m-%d"`.tar SYNDEM/README SYNDEM/Makefile SYNDEM/*_alleg SYNDEM/*.cpp SYNDEM/*.hpp SYNDEM/params* SYNDEM/p SYNDEM/ch SYNDEM/go SYNDEM/report SYNDEM/sim.plt SYNDEM/loc.* SYNDEM/pop.* SYNDEM/sched.* ; cd SYNDEM)
	make


##################### SynDEM src makefile ###########################

#################  Compiler Flags ##############################
CPP = g++
CPPFLAGS = -O2 -Wall

###############################################

%.o:%.cpp %.hpp
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ = Behavior.o Classroom.o Community.o Demographics.o Health.o \
	Hospital.o Household.o Infection.o Locations.o Neighborhood.o Office.o \
	Params.o Place.o Perceptions.o Person.o Population.o Profile.o \
	Random.o School.o Strain.o Workplace.o

SRC = $(OBJ:.o=.cpp)

HDR = $(OBJ:.o=.hpp)

FRED: Fred.cpp Fred.hpp $(OBJ)
	$(CPP) $(CPPFLAGS) Fred.cpp $(OBJ) -o fred

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f *.o *~ FRED out* trace* 

tar: clean
	cd ..
	tar cvf FRED.tar FRED

dist:
	make clean
	(cd ..; tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED/README \
	FRED/Makefile FRED/profiles.txt FRED/*.cpp FRED/*.hpp \
	FRED/params* FRED/p FRED/ch FRED/go FRED/report FRED/test_* \
	FRED/LICENSE FRED/sim.plt FRED/TEST/* ; cd FRED)
	make


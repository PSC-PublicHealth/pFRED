##################### SynDEM src makefile ###########################

#################  Compiler Flags ##############################
CPP = g++
CPPFLAGS = -O2 -Wall

###############################################

%.o:%.cpp %.hpp
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ = Classroom.o Community.o Disease.o Hospital.o Household.o Loc.o \
Neighborhood.o Office.o Params.o Place.o Person.o Pop.o Profile.o \
Random.o School.o Workplace.o

SRC = $(OBJ:.o=.cpp)

HDR = $(OBJ:.o=.hpp)

SYNDEM: Syndem.cpp Syndem.hpp $(OBJ)
	$(CPP) $(CPPFLAGS) Syndem.cpp $(OBJ) -o SYNDEM

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f *.o *~ SYNDEM out* trace*

tar: clean
	cd ..
	tar cvf SYNDEM.tar SYNDEM

dist:
	make clean
	(cd ..; tar cvf SYNDEM-`date +"%Y-%m-%d"`.tar SYNDEM/README \
	SYNDEM/Makefile SYNDEM/dc*.txt SYNDEM/profiles.txt SYNDEM/*.cpp SYNDEM/*.hpp \
	SYNDEM/params* SYNDEM/p SYNDEM/ch SYNDEM/go SYNDEM/report \
	SYNDEM/LICENSE SYNDEM/sim.plt ; cd SYNDEM)
	make


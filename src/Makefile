##################### SynDEM src makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 
#CPPFLAGS = -m64 -O2 -Wall
CPPFLAGS = -m64 -g -O0 -Wall

###############################################

%.o:%.c %.h
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ =   AgeMap.o Antiviral.o Antivirals.o Behavior.o Classroom.o Community.o Demographics.o Health.o \
	Hospital.o Household.o Infection.o Locations.o Neighborhood.o Office.o \
	Params.o Place.o Perceptions.o Person.o Population.o Profile.o \
	Random.o School.o Spread.o Strain.o Workplace.o

SRC = $(OBJ:.o=.cc)

HDR = $(OBJ:.o=.h)

FRED: Fred.cc Fred.h $(OBJ)
	$(CPP) $(CPPFLAGS) Fred.cc $(OBJ) -o FRED

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
	FRED/Makefile FRED/profiles.txt FRED/*.cc FRED/*.h \
	FRED/params* FRED/p FRED/ch FRED/go FRED/report FRED/test_* \
	FRED/LICENSE FRED/sim.plt FRED/TEST/* ; cd FRED)
	make


##################### SynDEM src makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 
CPPFLAGS = -m64 -O2 -Wall
# CPPFLAGS = -g -pg -O0 -Wall

###############################################

%.o:%.c %.h
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ =   Decision.o Policy.o Manager.o \
        AV_Decisions.o AV_Policies.o AV_Manager.o AV_Status.o \
        AgeMap.o VaccineStatus.o VaccineDose.o Vaccine.o Vaccines.o \
        Vaccine_Priority_Decisions.o Vaccine_Priority_Policies.o Vaccine_Manager.o \
	Antiviral.o Antivirals.o Behavior.o Classroom.o Community.o Demographics.o Health.o \
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


##################### FRED Makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 

# use this for development:
# CPPFLAGS = -g -m64 -O3 -fast #-Wall

# Use this for production:
CPPFLAGS = -m64 -O3 -DNDEBUG -fast #-Wall

# Use this for unit testing:
# CPPFLAGS = -DUNITTEST -m64 -O3 #-Wall

###############################################

%.o:%.c %.h
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ =   Global.cc Decision.o Policy.o Manager.o \
        AV_Decisions.o AV_Policies.o AV_Manager.o AV_Health.o \
        Age_Map.o Timestep_Map.o Vaccine_Health.o Vaccine_Dose.o Vaccine.o Vaccines.o \
        Vaccine_Priority_Decisions.o Vaccine_Priority_Policies.o Vaccine_Manager.o \
	Antiviral.o Antivirals.o Behavior.o Classroom.o Community.o Demographics.o Health.o \
        Hospital.o Household.o Infection.o Locations.o Neighborhood.o Office.o \
	Params.o Place.o Cognition.o Perceptions.o Person.o Population.o Profile.o \
	Random.o School.o Spread.o Strain.o Workplace.o Health_Belief_Model.o Random_Cognitive_Model.o

SRC = $(OBJ:.o=.cc)

HDR = $(OBJ:.o=.h)

all:	FRED

FRED: Fred.cc Fred.h $(OBJ)
	$(CPP) $(CPPFLAGS) Fred.cc $(OBJ) -o FRED

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f *.o *~ FRED

tar: clean
	cd ..
	tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED

dist:
	make clean
	(cd ..; tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED/README \
	FRED/Makefile FRED/*.txt FRED/*.cc FRED/*.h FRED/params* \
	FRED/bin/* FRED/LICENSE FRED/sim.plt FRED/TEST/* ; cd FRED)
	make

tags:
	find . -name \*.[ch]* | xargs etags

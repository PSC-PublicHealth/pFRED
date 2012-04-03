##################### FRED Makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 

# use this for development:
# CPPFLAGS = -g -m64 -pg -O3 -Wall

# use one of these for profiling
# CPPFLAGS = -g -m64 -O3 # -DNDEBUG
# CPPFLAGS = -pg -m64 -O2 -DNDEBUG -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-omit-frame-pointer

# Use this for production:
CPPFLAGS = -g -m64 -O3 -DNDEBUG #-fast #-Wall

# Use this for unit testing:
# CPPFLAGS = -DUNITTEST -m64 -O3 #-Wall

#################  MD5 Program ##############################

UNIX	:= $(shell uname)
ifeq ($(UNIX), Linux)
MD5SUM	:= md5sum
else
MD5SUM	:= md5 -q
endif

###############################################

# SUNDIALS ode solvers
#SUNDIALS_DIR := sundials/osx
#CVODE_LIB=/usr/local/lib/libsundials_cvode.a
#NVECS_LIB=/usr/local/lib/libsundials_nvecserial.a

#IFLAGS := -I$(SUNDIALS_DIR)/include
#LFLAGS := -L$(SUNDIALS_DIR)/lib -lsundials_cvode -lsundials_nvecserial -lm

#CPPFLAGS = -g -m64 -O3 $(IFLAGS) $(LFLAGS)

###############################################

%.o:%.c %.h
	$(CPP) $(CPPFLAGS) -c $< $(INCLUDES)

OBJ =   Fred.o Global.o Age_Map.o Timestep_Map.o Utils.o Params.o Date.o Random.o \
	Geo_Utils.o Cell.o Grid.o Large_Grid.o Large_Cell.o Small_Grid.o Small_Cell.o Travel.o \
	Decision.o Policy.o Manager.o \
	Antiviral.o Antivirals.o AV_Decisions.o AV_Policies.o AV_Manager.o AV_Health.o \
        Vaccine_Health.o Vaccine_Dose.o Vaccine.o Vaccines.o \
        Vaccine_Priority_Decisions.o Vaccine_Priority_Policies.o Vaccine_Manager.o \
	Person.o Place.o Place_List.o Population.o \
	Activities.o Attitude.o Behavior.o Demographics.o Health.o Perceptions.o \
        Classroom.o Hospital.o Household.o Neighborhood.o Office.o School.o Workplace.o \
	Disease.o Infection.o Epidemic.o \
	EvolutionFactory.o Evolution.o Strain.o Trajectory.o StrainTable.o \
	Multistrain_Timestep_Map.o \
	IntraHost.o DefaultIntraHost.o FixedIntraHost.o \
	Abstract_Grid.o Abstract_Cell.o \
	Seasonality_Timestep_Map.o Seasonality.o
	# ODEIntraHost.o ODE.o

SRC = $(OBJ:.o=.cc)

HDR = $(OBJ:.o=.h)

MD5 := FRED.md5

all: FRED FRED.tar.gz $(MD5)

FRED: $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o FRED
	cp FRED ../bin

DEPENDS: $(SRC) $(HDR)
	$(CPP) -MM $(SRC) > DEPENDS

include DEPENDS

FRED.tar.gz: $(SRC) $(HDR)
	tar -czf FRED.tar.gz $(HDR) $(SRC) Makefile

FRED.md5 : FRED.tar.gz
	$(MD5SUM) $< > $@

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f gmock.a gmock_main.a *.o FRED ../bin/FRED *~
	(cd ../region; make clean)
	(cd ../tests; make clean)

tar: clean
	cd ..
	tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED

dist:
	make clean
	(cd ../..; tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED/src/Makefile \
	FRED/src/*.txt FRED/src/*.cc FRED/src/*.h FRED/src/params* \
	FRED/bin/* FRED/src/OUT.RT/* ; cd FRED/src)
	make

VER = 1.0.1

release:
	make clean
	(cd ../..; tar cvzf FRED-V${VER}-`date +"%Y-%m-%d"`.tgz --exclude CVS \
	FRED/Makefile FRED/LICENSE FRED/bin FRED/doc FRED/input_files FRED/region/ \
	FRED/src/Makefile FRED/src/*.h  FRED/src/*.cc FRED/src/mt*.c FRED/tests)

tags:
	find . -name \*.[ch]* | xargs etags

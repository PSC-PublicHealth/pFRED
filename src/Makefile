##################### FRED Makefile ###########################

SRC_DIR = $(shell pwd)
PRNG_DIR = $(SRC_DIR)/PRNG__Mersenne_Twister
DSFMT_DIR = $(PRNG_DIR)/dSFMT-src-2.2
SQLITE_DIR = $(SRC_DIR)/sqlite
SNAPPY_DIR = $(SRC_DIR)/snappy
SNAPPY_INC_DIR = $(SNAPPY_DIR)/include
SNAPPY_HDR = $(SNAPPY_INC_DIR)/snappy.h 

INCLUDE_DIRS = -I$(DSFMT_DIR) -I$(SQLITE_DIR) -I$(SNAPPY_INC_DIR)


################   Multithreading   ###############################
# For multithreading, NCPU should be set here or as an argument to make.
# NCPU must be greater than or equal to the maximum number of threads that will
# be used by OpenMP.  The number of threads used by OpenMP can be set at runtime
# with the environment variable OMP_NUM_THREADS.  The value used for OMP_NUM_THREADS
# must be less than or equal the value of NCPU set during compilation.
# If NCPU > omp_get_max_threads, some relatively small amount of memory will be
# wasted, but it's harmless otherwise.
#
# NCPU is defined to be 1 in Global.h if value not set here 
#
# Autodetect all available processors for linux or mac
# Fall back to single-threaded operation
OS := $(shell uname)
ifeq ($(OS),Linux)
NCPU ?= $(shell grep -c ^processor /proc/cpuinfo)
SNAPPY_LIB = $(SNAPPY_DIR)/lib/libsnappy.so
M64 = -m64
else ifeq ($(shell echo $(OS) | cut -f1 -d_),CYGWIN)
NCPU ?=  $(shell grep -c ^processor /proc/cpuinfo)
SNAPPY_LIB = $(SNAPPY_DIR)/lib/libsnappy.la 
else ifeq ($(OS),Darwin)
NCPU ?= $(shell sysctl -n hw.ncpu)
SNAPPY_LIB = $(SNAPPY_DIR)/lib/libsnappy.dylib
M64 = -m64
endif
################   Compiler Flags   ##############################
FRED_EXECUTABLE_NAME = FRED

CPP = g++ 
# CPP = /Developer/usr/bin/g++

LOGGING_PRESET_0 = 
LOGGING_PRESET_1 = -DFREDWARNING
LOGGING_PRESET_2 = -DFREDSTATUS -DFREDWARNING
LOGGING_PRESET_3 = -DFREDVERBOSE -DFREDSTATUS -DFREDWARNING -DFREDDEBUG

DSFMT_CPPFLAGS = -g -O3 -DDSFMT_MEXP=19937 -DDSFMT_DO_NOT_USE_OLD_NAMES -finline-functions -fomit-frame-pointer -DNDEBUG \
-fno-strict-aliasing --param max-inline-insns-single=1800

DSFMT_SRC = $(DSFMT_DIR)/dSFMT.c
DSFMT_HDR = $(DSFMT_DIR)/dSFMT.h $(DSFMT_DIR)/dSFMT-params19937.h

# use this for development:
## CPPFLAGS = -g -O2 $(LOGGING_PRESET_3) -Wall

# use one of these for profiling
# CPPFLAGS = -g $(M64) -O3 $(LOGGING_PRESET_3) # -DNDEBUG
# CPPFLAGS = -pg $(M64) -O2 -DNDEBUG $(LOGGING_PRESET_3) -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-omit-frame-pointer
# Use one of these for production:

## Use this to run with multiple threads
CPPFLAGS = -g $(M64) -O3 -fopenmp $(LOGGING_PRESET_3) -DNCPU=$(NCPU) -fno-omit-frame-pointer


## Use this to make reproducible serial runs
# CPPFLAGS = -g $(M64) -O3 $(LOGGING_PRESET_3) -DNCPU=1 #-fast #-Wall

CPPFLAGS += $(INCLUDE_DIRS)

CXX = $(CPP)
CXXFLAGS = $(CPPFLAGS)

LDFLAGS = -L$(SNAPPY_DIR)/lib/

LFLAGS = $(SNAPPY_DIR)/lib/libsnappy.a

#################  MD5 Program ##############################

UNIX	:= $(shell uname)
ifeq ($(UNIX), Linux)
MD5SUM	:= md5sum
else ifeq ($(shell echo $(UNIX) | cut -f1 -d_),CYGWIN)
MD5SUM := md5sum
else
MD5SUM	:= md5 -q
endif

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
	Seasonality_Timestep_Map.o Seasonality.o \
	Past_Infection.o MSEvolution.o FergEvolution.o Piecewise_Linear.o \
	Compression.o
	# ODEIntraHost.o ODE.o

SRC = $(OBJ:.o=.cc)

HDR = $(OBJ:.o=.h)

MD5 := FRED.md5

all: FRED FRED.tar.gz fsz $(MD5)

FRED: $(SNAPPY_LIB) $(OBJ) dSFMT.o sqlite3.o
	$(CPP) -o $(FRED_EXECUTABLE_NAME) $(CPPFLAGS) $(INCLUDE_FLAGS) $(LDFLAGS) $(OBJ) dSFMT.o sqlite3.o $(LFLAGS) -ldl
	cp FRED ../bin

dSFMT.o:
	$(CPP) $(DSFMT_CPPFLAGS) $(DSFMT_SRC) -c $(DSFMT_HDR) 

sqlite3.o:
	cd $(SQLITE_DIR); tar --strip-components=1 -xzvf sqlite-autoconf-3071502.tar.gz; ./configure; make; cp sqlite3.o $(SRC_DIR)

fsz: $(SNAPPY_LIB) Compression.o Global.h fsz.cc
	$(CPP) -o fsz $(CPPFLAGS) $(INCLUDE_FLAGS) $(LDFLAGS) Compression.o $(LFLAGS) fsz.cc
	cp fsz ../bin

DEPENDS: $(SRC) $(HDR)
	$(CPP) -MM $(SRC) > DEPENDS

include DEPENDS

$(SNAPPY_LIB):
	cd $(SNAPPY_DIR); tar --strip-components=1 -xzvf snappy-1.0.5.tar.gz; ./configure --prefix=$(SNAPPY_DIR); make && make install;


FRED.tar.gz: $(SRC) $(HDR)
	tar -czf FRED.tar.gz $(HDR) $(SRC) Makefile

FRED.md5 : FRED.tar.gz
	$(MD5SUM) $< > $@

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f gmock.a gmock_main.a *.o FRED ../bin/FRED fsz ../bin/fsz *~
	(cd ../region; make clean)
	(cd ../tests; make clean)
	(cd $(SNAPPY_DIR); make clean)
	(cd $(SNAPPY_DIR); rm -r lib include share)
	(cd $(SQLITE_DIR); make clean)

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
	rm -f gmock.a gmock_main.a *.o FRED ../bin/FRED *~

tags:
	find . -name \*.[ch]* | xargs etags

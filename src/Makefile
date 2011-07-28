##################### FRED Makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 

# use this for development:
CPPFLAGS = -g -pg -O3 -Wall # -fast #-Wall

# use this for profiling
# CPPFLAGS = -g -m64 -O3 # -DNDEBUG

# Use this for production:
# CPPFLAGS = -m64 -O3 -DNDEBUG -fast #-Wall

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

OBJ =   Fred.o Global.o Utils.o Decision.o Policy.o Manager.o \
        AV_Decisions.o AV_Policies.o AV_Manager.o AV_Health.o \
        Age_Map.o Timestep_Map.o Vaccine_Health.o Vaccine_Dose.o Vaccine.o Vaccines.o \
        Vaccine_Priority_Decisions.o Vaccine_Priority_Policies.o Vaccine_Manager.o \
	Antiviral.o Antivirals.o Health_Belief_Model.o Random_Behavior_Model.o \
	Activities.o Classroom.o Behavior.o Demographics.o Epidemic.o Health.o \
        Hospital.o Household.o Infection.o Neighborhood.o Office.o \
	Params.o Perceptions.o Person.o Place.o Place_List.o Population.o Profile.o Random.o \
	School.o Disease.o Workplace.o \
	Geo_Utils.o Cell.o Grid.o Large_Grid.o Large_Cell.o Small_Grid.o Small_Cell.o \
	Date.o Person_Event_Interface.o Trajectory.o StrainTable.o \
	EvolutionFactory.o Evolution.o Strain.o \
	Multistrain_Timestep_Map.o \
	IntraHost.o DefaultIntraHost.o FixedIntraHost.o  # ODEIntraHost.o ODE.o

SRC = $(OBJ:.o=.cc)

HDR = $(OBJ:.o=.h) Behavior_Model.h

POP := $(wildcard *_pop.txt)
LOC := $(wildcard *_loc.txt)
MD5 := FRED.md5 $(POP:.txt=.md5) $(LOC:.txt=.md5)

all: FRED FRED.tar.gz $(MD5)

FRED: $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o FRED
	cp FRED ../bin

DEPENDS: $(SRC) $(HDR)
	$(CPP) -MM $(SRC) > DEPENDS

include DEPENDS

FRED.tar.gz: $(SRC) $(HDR)
	tar -czf FRED.tar.gz $(HDR) $(SRC) Makefile params* *.txt

FRED.md5 : FRED.tar.gz
	$(MD5SUM) $< > $@

%_pop.md5 : %_pop.txt
	$(MD5SUM) $< > $@

%_loc.md5 : %_loc.txt
	$(MD5SUM) $< > $@


############################# Google Test/Mock ################################

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file, or if you want to use
# a copy of Google Test at a different location.
GTEST_DIR = gmock/gtest

# Points to the root of Google Mock, relative to where this file is.
# Remember to tweak this if you move this file.
GMOCK_DIR = gmock

# Where to find user code.
USER_DIR = TestSuite

# Flags passed to the preprocessor. (Uncomment if testing)
# CPPFLAGS += -I$(GTEST_DIR)/include -I$(GMOCK_DIR)/include

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# All Google Mock headers. Note that all Google Test headers are
# included here too, as they are #included by Google Mock headers.
# Usually you shouldn't change this definition.
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

# Builds gmock.a and gmock_main.a.  These libraries contain both
# Google Mock and Google Test.  A test should link with either gmock.a
# or gmock_main.a, depending on whether it defines its own main()
# function.  It's fine if your test only uses features from Google
# Test (and not Google Mock).

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

# For simplicity and to avoid depending on implementation details of
# Google Mock and Google Test, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Mock and
# Google Test compile fast and for ordinary users their source rarely
# changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GTEST_DIR)/src/gtest-all.cc

gmock-all.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock-all.cc

gmock_main.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) \
            -c $(GMOCK_DIR)/src/gmock_main.cc

gmock.a : gmock-all.o gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gmock_main.a : gmock-all.o gtest-all.o gmock_main.o
	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.

gmock_test.o : $(GMOCK_DIR)/test/gmock_test.cc $(GMOCK_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/gmock_test.cc

gmock_test : gmock_test.o gmock_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

################################### Tests #####################################

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TEST_OBJS = TestSuite/Demographics/DemographicsTest.cc

$(TEST_OBJS): TestSuite/*.h

TESTS: TestSuite/TestSuite.o $(TEST_OBJS) $(OBJ) gmock.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o TestSuite/TestSuite

##############################################

print:
	enscript $(SRC) $(HDR)

clean:
	rm -f $(TESTS) gmock.a gmock_main.a *.o FRED *~

tar: clean
	cd ..
	tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED

dist:
	make clean
	(cd ../..; tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED/src/Makefile \
	FRED/src/*.txt FRED/src/*.cc FRED/src/*.h FRED/src/params* \
	FRED/bin/* FRED/src/OUT.RT/* ; cd FRED/src)
	make

tags:
	find . -name \*.[ch]* | xargs etags

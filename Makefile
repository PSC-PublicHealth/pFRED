##################### FRED Makefile ###########################

#################  Compiler Flags ##############################
CPP = g++ 

# use this for development:
CPPFLAGS = -g -m64 -O3 -fast #-Wall

# use this for profiling
# CPPFLAGS = -g -m64 -O3 -DNDEBUG

# Use this for production:
# CPPFLAGS = -m64 -O3 -DNDEBUG -fast #-Wall

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
	(cd ..; tar cvf FRED-`date +"%Y-%m-%d"`.tar FRED/README \
	FRED/Makefile FRED/*.txt FRED/*.cc FRED/*.h FRED/params* \
	FRED/bin/* FRED/LICENSE FRED/sim.plt FRED/TEST/* ; cd FRED)
	make

tags:
	find . -name \*.[ch]* | xargs etags

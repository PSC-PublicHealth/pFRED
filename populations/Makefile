##################### FRED Populations Makefile ###########################
#
#
# The following synthetic population are available:
#
# 2010_ver1_42003 = Allegheny_County_PA_2010_ver1
# 2010_ver1_42065 = Jefferson_County_PA_2010_ver1
# 2005_2009_ver2_42003 = Allegheny_County_PA_V2 (deprecated)
# 2005_2009_ver2_42065 = Jefferson_County_PA_V2 (deprecated)
#

FRED_HOME ?= $(CURDIR)/..
POPS = 2010_ver1_42003 2010_ver1_42065 2005_2009_ver2_42003 2005_2009_ver2_42065
OS := $(shell uname)

all: $(POPS)

# To create a population directory, run "make X"
% :: %.zip
	unzip $< -d $@
ifneq ($(shell echo $(OS) | cut -f1 -d_),CYGWIN)
	(cd $@; $(FRED_HOME)/bin/fsz -c $@_synth_people.txt > $@_synth_people.txt.fsz; cd ..)
endif

# To create a zip file, run "make X.zip"
%.zip: $(basename %)
	(cd $< ; zip -rj $@ *.txt ; mv $@ .. ; cd ..)

clean:
	rm -rf $(POPS)








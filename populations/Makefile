##################### FRED Populations Makefile ###########################
#
#
# The following synthetic population are available:
#
# 2000_ver1_42003 = ALLEGHENY_COUNTY_V1
# 2005_2009_ver2_42003 = ALLEGHENY_COUNTY_V2
# 2005_2009_ver2_42 = PENNSYLVANIA_V2
#

POPS = 2005_2009_ver2_42003 # 2005_2009_ver2_42 2000_ver1_42003

all: $(POPS)

# To create a population directory, run "make X"
% :: %.zip
	unzip $< -d $@

# To create a zip file, run "make X.zip"
%.zip: $(basename %)
	(cd $< ; zip -rj $@ *.txt ; mv $@ .. ; cd ..)

clean:
	rm -rf $(POPS)








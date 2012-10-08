##################### FRED Populations Makefile ###########################

POPS = 2005_2009_ver2_42003 2000_ver1_42003 # tw

# use these for file download from the RTI web site
2005_2009_ver2_42003:
	unzip 2005_2009_ver2_42003.zip -d 2005_2009_ver2_42003 

2005_2009_ver2_42003.zip: 2005_2009_ver2_42003
	rm -f 2005_2009_ver2_42003/*~
	zip -r tw tw/tw* tw/README*
	zip -r 2005_2009_ver2_42003 2005_2009_ver2_42003/2005_2009_ver2_42003* 2005_2009_ver2_42003/README*

# these are for population directories created at Pitt
2000_ver1_42003:
	unzip 2000_ver1_42003.zip

2000_ver1_42003.zip: 2000_ver1_42003
	rm -f 2000_ver1_42003/*~
	zip -r tw tw/tw* tw/README*
	zip -r 2000_ver1_42003 2000_ver1_42003/2000_ver1_42003* 2000_ver1_42003/README*

tw:
	unzip tw.zip

tw.zip: tw
	rm -f tw/*~
	zip -r tw tw/tw* tw/README*

clean:
	rm -rf 2005_2009_ver2_42003 2000_ver1_42003







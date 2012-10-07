##################### FRED Populations Makefile ###########################

POPS = 2005_2009_ver2_42003 2000_ver1_42003 # tw

2005_2009_ver2_42003:
	unzip 2005_2009_ver2_42003.zip -d 2005_2009_ver2_42003 

2005_2009_ver2_42003.zip: 2005_2009_ver2_42003
	zip -r $@ $^/$^* 

2000_ver1_42003:
	unzip 2000_ver1_42003.zip -d 2000_ver1_42003 

2000_ver1_42003.zip: 2000_ver1_42003
	zip -r $@ $^/$^* 

clean:
	rm -rf 2005_2009_ver2_42003 2000_ver1_42003

tw:
	unzip tw.zip -d tw 

tw.zip: tw
	zip -r $@ $^/$^* 






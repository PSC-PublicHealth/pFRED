##################### FRED Makefile ###########################

DIRS = bin doc input_files src populations tests

all:
	@for i in $(DIRS); do \
		echo $$i; \
		(cd $$i; make); \
	done

clean:
	@for i in $(DIRS); do \
		echo $$i; \
		(cd $$i; make clean); \
	done

VER = 1.0.7

release:
	make clean
	(cd ..; tar cvzf FRED-V${VER}-`date +"%Y-%m-%d"`.tgz \
	--exclude CVS --exclude '*~' --exclude '\.*' \
	FRED/Makefile FRED/LICENSE FRED/bin FRED/doc FRED/input_files \
	FRED/populations/2005_2009_ver2_42003.zip FRED/populations/Makefile \
	FRED/src FRED/tests)

tar: clean
	cd ..
	tar cvsf FRED-`date +"%Y-%m-%d"`.tgz FRED --exclude RESULTS





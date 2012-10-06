##################### FRED Makefile ###########################

all:
	(cd populations; make)
	(cd src; make)
	(cd tests; make)




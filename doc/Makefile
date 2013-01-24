###########################################
#
# FRED Makefile
#

all: FRED_UserGuide.pdf
	

FRED_UserGuide.pdf: FRED_UserGuide.rst FRED.css
	rst2pdf FRED_UserGuide.rst -o FRED_UserGuide.pdf --repeat-table-rows -s FRED.css

clean:
	

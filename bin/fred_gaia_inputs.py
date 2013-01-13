#! /usr/bin/env python

import os,sys,string
import math
import optparse
from fred import FRED,FRED_RUN,FRED_Infection_Record,FRED_Locations_Set,FRED_People_Set
from gaia import GAIA,ConfInfo,PlotInfo,Constants,computeBoundaries,computeColors
import time

POP_DENSITY_STYLE_NUMBER = 2
INC_STYLE_NUMBER = 3

###############################################################################################
### Grid Class definititions
                
class Grid:
    def __init__(self,bounding_box,increment):
        self.increment = increment
        self.bounding_box = bounding_box

        self.number_lon = int(((self.bounding_box[3]-self.bounding_box[1])/self.increment))+1
        self.number_lat = int(((self.bounding_box[2]-self.bounding_box[0])/self.increment))+1

        self.data = [0 for x in range(self.number_lon*self.number_lat)]
    

    def indexes(self,latitude,longitude):
        lonind = int(math.floor((longitude - self.bounding_box[1])/self.increment))
        latind = int(math.floor((latitude  - self.bounding_box[0])/self.increment))
        return lonind*self.number_lat + latind

    def __div__(self,value):
        return [ float(x/value) for x in self.data ]

    def writeToGaiaFile(self,file,styleID=None):
        for i in range(0,self.number_lon):
            for j in range(0,self.number_lat):
                lonpoint = self.bounding_box[1] + float(i)*self.increment +\
                           self.increment/2.0
                latpoint = self.bounding_box[0] + float(j)*self.increment +\
                           self.increment/2.0
                grid_coord = i*self.number_lat + j
                if(self.data[grid_coord] != 0):
                    if styleID:
                        file.write("lonlat %10.5f %10.10f %10d:%d\n"\
                                   %(latpoint,lonpoint,self.data[grid_coord],styleID))
                    else:
                        file.write("lonlat %10.5f %10.10f %10d\n"\
                                   %(latpoint,lonpoint,self.data[grid_coord]))
        
    def writeStyleFile(self,file,radius=6.0,startColor="255.255.0.0",
                       endColor="255.0.0.255",numbounds=10,styleID=None):
        boundaries = computeBoundaries(self.data,numbounds)
        colors = computeColors(startColor,endColor,numbounds)

        if styleID:
            file.write("id=%d\n"%styleID)
        for boundary in boundaries:
            file.write('%s %g %g %g\n'%(colors[boundaries.index(boundary)],radius,boundary[0],boundary[1]))

class PopDenGrid(Grid):
    def __init__(self,bounding_box_,increment_,fred_locations_):
        Grid.__init__(self,bounding_box_,increment_)

        assert(isinstance(fred_locations_,FRED_Locations_Set))

        for location in fred_locations_.locations.keys():
            place = fred_locations_.locations[location]
            lon = place.lon
            lat = place.lat
            grid_coords = self.indexes(lat,lon)
            self.data[grid_coords] = self.data[grid_coords]\
                                     + place.size
    def writeToGaiaFile(self,file):
        Grid.writeToGaiaFile(self,file,POP_DENSITY_STYLE_NUMBER)

    def writeStyleFile(self,file,radius=6.0,numbounds=10):
        Grid.writeStyleFile(self,file,radius,"255.200.200.200","255.0.0.0",numbounds,
                            POP_DENSITY_STYLE_NUMBER)

class IncidenceGrid(Grid):
    def __init__(self,bounding_box_,increment_,fred_locations_,fred_population_,infections_):
        Grid.__init__(self,bounding_box_,increment_)

        assert(isinstance(fred_locations_,FRED_Locations_Set))
        assert(isinstance(fred_population_,FRED_People_Set))

        for inf_rec in infections_:
            inf_id = int(inf_rec.infected_id)
            household = fred_population_.people[inf_id].household
            place = fred_locations_.locations[household]
            lon = place.lon
            lat = place.lat
            grid_coords = self.indexes(lat,lon)
            self.data[grid_coords] = self.data[grid_coords] + 1

    def writeToGaiaFile(self,file):
        Grid.writeToGaiaFile(self,file,INC_STYLE_NUMBER)

    def writeStyleFile(self,file,radius=6.0,numbounds=10):
        Grid.writeStyleFile(self,file,radius=radius,numbounds=numbounds,
                            styleID=INC_STYLE_NUMBER)

class AveIncidenceGrid(IncidenceGrid):
    def __init__(self,bounding_box_,increment_,fred_locations_,fred_population_,infections_dict_):
        emptyList = []
        IncidenceGrid.__init__(self,bounding_box_,increment_,fred_locations_,fred_population_, emptyList)

        for infections_file_key in infections_dict_.keys():
            infections_file = infections_dict_[infections_file_key]
            #fill the infectious places
            for inf_rec in infections_file:
                inf_id = int(inf_rec.infected_id)
                household = fred_population_.people[inf_id].household
                place = fred_locations_.locations[household]
                lon = place.lon
                lat = place.lat
                grid_coords = self.indexes(lat,lon)
                self.data[grid_coords] = self.data[grid_coords] + 1
            nruns = len(infections_dict_.keys())
            self.data = [ float(x/float(nruns)) for x in self.data ]

          
###############################################################################################


if __name__ == '__main__':
    parser = optparse.OptionParser(usage="""
    %prog [--help][-k key][-r run_number][-t vis_type]
    -k or --key   Fred Key
    -r or --run   The run number of vizualize
                  ave = produce an average result
                  all = produce an input for all runs
    -t or --type  Type of GAIA visualization
		  hous_den   = Household Density Plot
                  inc_anim   = incidence animation
                  inc_static = incidence map
    -p or --pop   FRED Population file to use (if blank will try to use the one from input)
    -l or --loc   FRED Location file to use (if blank will try to use the one from input)
    -g or --grid  Size of grid points to plot
    -w or --time  Turn on profiling
    """)

    parser.add_option("-k","--key",type="string",
                      help="The FRED key for the run you wish to visualize")
    parser.add_option("-r","--run",type="string",
                      help="The number of the run you would like to visualize (number,ave, or all)",
                      default=1)
    parser.add_option("-t","--type",type="string",
                      help="The type of GAIA Visualization you would like (inc_anim,inc_static)",
                      default="inc_static")
    parser.add_option("-p","--pop",type="string",
                      help="The FRED Population File to use")
    parser.add_option("-l","--loc",type="string",
                      help="The FRED Location File to use")

    parser.add_option("-g","--grid",type="float",
                      help="The size of grid points",
                      default=0.002)
    parser.add_option("-w","--time",action="store_true",
                      default=False)
    

    opts,args=parser.parse_args()

    #### Initialize the FRED SIMS
    fred = FRED()
    
    key = opts.key
    run = opts.run
    try:
        int(run)
    except:
        if run != "ave" and run != "all":
            print "Invalid parameter for run, needs to be an integer, all or ave"
            sys.exit(1)

    valid_types = ["inc_static","inc_anim",'pop_den','test_grid']
    if opts.type not in valid_types:
        print "Invalid or unsupported type parameter, needs to be inc_static or inc_anim"
        sys.exit(1)
    vizType = opts.type

   ### Find the FRED run in this SIMS
    fred_run = FRED_RUN(fred,key)

    ### Find the Population File

    time1 = time.time()
    synth_pop_dir = fred_run.get_param("synthetic_population_directory")
    synth_pop_id = fred_run.get_param("synthetic_population_id")
    synth_pop_prefix = os.path.expandvars(synth_pop_dir + "/" + synth_pop_id + "/" + synth_pop_id)
    synth_pop_filename = synth_pop_prefix + "_synth_people.txt"
    synth_household_filename = synth_pop_prefix + "_synth_households.txt"
    
    try:
        open(synth_pop_filename,"r")
    except IOError:
        print "Problem opening FRED population file " + synth_pop_filename
        sys.exit(1)
        
    try:
        open(synth_household_filename,"r")
    except IOError:
        print "Problem opening FRED location file " + loc_file
        
    if vizType == "test_grid":
        gaia_bounding_box = (0.0,0.0,100.0,100.0)
        gaia_grid = Grid(gaia_bounding_box,0.1)
        print "Coords for 2.5445,55.3233 = " + str(gaia_grid.indexes(2.5445,55.3233))
        print "Coords for 0.1,0.2 = " + str(gaia_grid.indexes(0.1,0.2))
        print "Coords for 100.0,100.0 = " + str(gaia_grid.indexes(100.0,100.0))
        sys.exit()
        
    fred_population = FRED_People_Set(synth_pop_filename)
    fred_locations_households = FRED_Locations_Set(synth_household_filename,"households")
    time2 = time.time()
    if opts.time: print "Time to Read Synthetic Population = %g sec"%(time2-time1)
    
## CREATE the GAIA Input
    gaia_bounding_box = fred_locations_households.bounding_box()
    if vizType == "inc_static":
        if opts.run == "all" or opts.run == "ave":
            #print "all"
            fred_run.load_infection_files()
        else:
            fred_run.load_infection_files(int(opts.run))

    if vizType == "inc_anim":
        fred_run.load_infection_files(1)
    
    if vizType == "inc_static":
        ## fill the population density first, as it is constant
        gaia_grid_pop = PopDenGrid(gaia_bounding_box,opts.grid,fred_locations_households)
        gaia_style_pop_file = "fred_gaia_pop_style_"+ str(fred_run.key) + ".txt"
       
        with open(gaia_style_pop_file,"wb") as f:
            gaia_grid_pop.writeStyleFile(f,radius=1.0)
                
        if opts.run == "ave":
            gaia_input_file = "fred_gaia_" + str(fred_run.key) + ".ave.txt" 
            gaia_style_inc_file = "fred_gaia_inc_style_"+ str(fred_run.key) + ".ave.txt"
            gaia_grid_inc = AveIncidenceGrid(gaia_bounding_box,opts.grid,
                                            fred_locations_households,fred_population,
                                            fred_run.infections_files)
            with open(gaia_input_file,"w") as f:
                ### Write out the county fips elements
                for countyFips in fred_locations_households.countyList:
                    f.write("USFIPS st%2s.ct%3s.tr* -1\n"%(countyFips[0:2],countyFips[2:5]))
                ### Write the population density grid out
                gaia_grid_pop.writeToGaiaFile(f)
 
                ### Write the incidence grid
                gaia_grid_inc.writeToGaiaFile(f)
                
                with open(gaia_style_inc_file,"wb") as f:
                    gaia_grid_inc.writeStyleFile(f,1.0,numbounds=10)
                
                plotInfo = PlotInfo(gaia_input_file,None,[gaia_style_pop_file,gaia_style_inc_file],
                                    False,"png",None,None,None,-1.0,-1.0,
                                    0,1000,False,None,-1.0,-1.0,"255.255.255.255",None,2.0,None,None)
                    
                time1 = time.time()
                gaia = GAIA(plotInfo)
                time2 = time.time()
                if opts.time: print "Time to produce GAIA object = %g sec"%(time2 - time1)
                    
                time1 = time.time()
                gaia.call()
                time2 = time.time()
                if opts.time: print "Time for GAIA to produce visualization = %g sec"%(time2 - time1)
        else:
            for infections_file_key in fred_run.infections_files.keys():
                infections_file = fred_run.infections_files[infections_file_key]
                gaia_grid_inc = IncidenceGrid(gaia_bounding_box,opts.grid,fred_locations_households,
                                              fred_population,infections_file)
                gaia_input_file = "fred_gaia_" + str(fred_run.key) + "."\
                                  + str(infections_file_key) + ".txt"
            
                gaia_style_inc_file = "fred_gaia_inc_style_"+ str(fred_run.key) + "."\
                                      + str(infections_file_key) + ".txt"
               
                with open(gaia_input_file,"w") as f:
                ### Write out the county fips elements
                    for countyFips in fred_locations_households.countyList:
                        f.write("USFIPS st%2s.ct%3s.tr* -1\n"%(countyFips[0:2],countyFips[2:5]))
                ### Write the population density grid out
                    gaia_grid_pop.writeToGaiaFile(f)
                    gaia_grid_inc.writeToGaiaFile(f)

                with open(gaia_style_inc_file,"wb") as f:
                    gaia_grid_inc.writeStyleFile(f,1.0,numbounds=10)

                plotInfo = PlotInfo(gaia_input_file,None,[gaia_style_pop_file,gaia_style_inc_file],
                                    False,"png",None,None,None,-1.0,-1.0,
                                    0,1000,False,None,-1.0,-1.0,"255.255.255.255",None,2.0,None,None)

                time1 = time.time()
                gaia = GAIA(plotInfo)
                time2 = time.time()
                if opts.time: print "Time to produce GAIA object = %g sec"%(time2 - time1)

                time1 = time.time()
                gaia.call()
                time2 = time.time()
                if opts.time: print "Time for GAIA to produce visualization = %g sec"%(time2 - time1)

    if vizType == 'pop_den':
	gaia_grid = PopDenGrid(gaia_bounding_box,opts.grid,fred_locations_households)
	gaia_input_file = "fred_gaia_pop_den_"+str(fred_run.key) + ".txt"
        gaia_style_file = "fred_gaia_pop_den_"+str(fred_run.key) + ".style.txt"
        
        with open(gaia_input_file,"w") as f:
            for countyFips in fred_locations_households.countyList:
                f.write("USFIPS st%2s.ct%3s.tr* -1\n"%(countyFips[0:2],countyFips[2:5]))
            gaia_grid.writeToGaiaFile(f,3)
            
        ### Compute the Boundaries
        with open(gaia_style_file,"wb") as f:
            gaia_grid.writeStyleFile(f)
        plotInfo = PlotInfo(gaia_input_file,None,[gaia_style_file],False,"png",None,None,None,-1,-1,
                            0,5000,False,None,-1.0,128.0,"255.255.255.255",None,1.0,None,None)

        time1 = time.time()
        gaia = GAIA(plotInfo)
        time2 = time.time()
        if opts.time: print "Time to produce GAIA object = %g sec"%(time2 - time1)

        time1 = time.time()
        gaia.call()
        time2 = time.time()
        if opts.time: print "Time for GAIA to produce visualization = %g sec"%(time2 - time1)

    ## if vizType == "hous_den":
    ##     gaia_grid = Grid(gaia_bounding_box,opts.grid)
    ##     gaia_input_file = "fred_gaia_house_den_"+str(fred_run.key) + ".txt"
    ##     for location in fred_locations_households.locations.keys():
    ##         place = fred_locations_households.locations[location]
    ##         lon = place.lon
    ##         lat = place.lat
    ##         grid_coords = gaia_grid.indexes(lat,lon)
    ##         gaia_grid.data[grid_coords[1]][grid_coords[0]] = gaia_grid.data[grid_coords[1]][grid_coords[0]] + 1
    ##     with open(gaia_input_file,"w") as f:
    ##         for countyFips in fred_locations_households.countyList:
    ##             f.write("USFIPS st%2s.ct%3s.tr* -1\n"%(countyFips[0:2],countyFips[2:5]))
    ##         for i in range(0,gaia_grid.number_lon):
    ##             for j in range(0,gaia_grid.number_lat):
    ##                 lonpoint = gaia_grid.bounding_box[1] + float(i)*gaia_grid.increment + gaia_grid.increment/2.0
    ##                 latpoint = gaia_grid.bounding_box[0] + float(j)*gaia_grid.increment + gaia_grid.increment/2.0
    ##                 if(gaia_grid.data[i][j] != 0):
    ##                     f.write("lonlat %10.10f %10.10f %10d\n"%(latpoint,lonpoint,gaia_grid.data[i][j]))

    ##     plotInfo = PlotInfo(gaia_input_file,None,None,False,"png",None,"255.200.200.200","255.0.0.0",6.0,6.0,
    ##                         10,5000,False,None,-1.0,-1.0,"255.255.255.255",None,1.0,None,None)
    
    if vizType == "inc_anim":
        ### Get the popoulation density
        ## fill the population density first, as it is constant
        gaia_grid_pop = PopDenGrid(gaia_bounding_box,opts.grid,fred_locations_households)
        gaia_style_pop_file = "fred_gaia_pop_style_"+ str(fred_run.key) + ".txt"
        with open(gaia_style_pop_file,"wb") as f:
            gaia_grid_pop.writeStyleFile(f,radius=0.75)
            
        for infections_file_key in fred_run.infections_files.keys():
            infections_file = fred_run.infections_files[infections_file_key]
            #if fred_run.infections_files.index(infections_file) != int(opts.run): continue
        ## First find out how many days one needs to show
            time1 = time.time()
            max_day = -99999
            gaia_input_file = "fred_gaia_ts_" + str(fred_run.key) + "." \
                              + str(infections_file_key) + ".txt"
            gaia_style_inc_file = "fred_gaia_style_inc" + str(fred_run.key)\
                                  + "." + str(infections_file_key) + ".txt"
            for inf_rec in infections_file:
                day = inf_rec.day
                max_day = max(day,max_day)
                gaia_grids = []
            for i in range(0,max_day+1):
                gaia_grids.append(Grid(gaia_bounding_box,opts.grid))

            for inf_rec in infections_file:
                inf_id = int(inf_rec.infected_id)
                household = fred_population.people[inf_id].household
                place = fred_locations_households.locations[household]
                lon = place.lon
                lat = place.lat
                day = int(inf_rec.day)
                grid_coords = gaia_grids[day].indexes(lat,lon)
                gaia_grids[day].data[grid_coords] =\
                                    gaia_grids[day].data[grid_coords] + 1
            time2 = time.time()
            if opts.time: print "Time to grid the incidence = %g"%(time2-time1)

            time1 = time.time()
            with open(gaia_input_file,"w") as f:
                max_inc = -99999
                peak_day = 0
                for k in range(0,max_day+1):
                    gaia_grid = gaia_grids[k]
                    incSum = sum(gaia_grid.data)
                    if incSum > max_inc:
                        max_inc = incSum
                        #print "Day: %g IncSum: %g PeakDay %k"%(k,max_inc,peak_day
                        peak_day = k

                    for countyFips in fred_locations_households.countyList:
                        f.write("USFIPS st%2s.ct%3s.tr* -1 %g\n"%(countyFips[0:2],countyFips[2:5],k))
                    
                    for i in range(0,gaia_grid_pop.number_lon):
                        for j in range(0,gaia_grid_pop.number_lat):
                            lonpoint = gaia_grid_pop.bounding_box[1] + float(i)*gaia_grid_pop.increment +\
                                       gaia_grid_pop.increment/2.0
                            latpoint = gaia_grid_pop.bounding_box[0] + float(j)*gaia_grid_pop.increment +\
                                       gaia_grid_pop.increment/2.0
                            grid_coord= i*gaia_grid_pop.number_lat + j
                            if(gaia_grid_pop.data[grid_coord] != 0):
                                f.write("lonlat %10.5f %10.10f %10d %g:2\n"\
                                        %(latpoint,lonpoint,gaia_grid_pop.data[grid_coord],k))
                                
                    for i in range(0,gaia_grid.number_lon):
                        for j in range(0,gaia_grid.number_lat):
                            lonpoint = gaia_grid.bounding_box[1] +\
                                       float(i)*gaia_grid.increment + gaia_grid.increment/2.0
                            latpoint = gaia_grid.bounding_box[0] +\
                                       float(j)*gaia_grid.increment + gaia_grid.increment/2.0
                            grid_coords = i*gaia_grid.number_lat + j
                            if(gaia_grid.data[grid_coords] != 0):
                                f.write("lonlat %10.5f %10.5f %10d %10d:3\n"%(latpoint,lonpoint,\
                                                                            int(gaia_grid.data[grid_coords]),k))	
                time2 = time.time()
                if opts.time: print "Time to write gaia file = %g"%(time2-time1)
                time1 = time.time()
                boundaries_inc = computeBoundaries(gaia_grids[peak_day].data,5)
                colors_inc = computeColors("255.255.0.0","255.0.0.255",len(boundaries_inc))

                with open(gaia_style_inc_file,"wb") as f:
                    f.write('id=3\n')
                    for boundary in boundaries_inc:
                        f.write('%s 0.75 %g %g\n'%(colors_inc[boundaries_inc.index(boundary)],\
                                                  boundary[0],boundary[1]))
            
                time2 = time.time()
                if opts.time: print "Time to computer inc boudaries = %g"%(time2-time1)
                plotInfo = PlotInfo(gaia_input_file,None,[gaia_style_pop_file,gaia_style_inc_file],False,
                                    "gif","mpg",None,None,-1.0,-1.0,
                                    0,500,False,None,-1.0,12.0,"255.255.255.255",None,2.0,None,None)

                time1 = time.time()
                gaia = GAIA(plotInfo)
                time2 = time.time()
                if opts.time: print "Time to produce GAIA object = %g sec"%(time2 - time1)
                
                time1 = time.time()
                gaia.call()
                time2 = time.time()
                if opts.time: print "Time for GAIA to produce visualization = %g sec"%(time2 - time1)


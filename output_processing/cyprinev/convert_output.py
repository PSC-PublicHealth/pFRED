import pandas as pd
import numpy as np
import ujson, yaml, time, bz2, gzip, os
from collections import OrderedDict, defaultdict
import pyximport
pyximport.install(reload_support=True)
#import cyprinev.count_events as count_events
import count_events


class OutputCollection(object):

    def default_config():
        return open('default_group_config.yaml','r')

    def __init__(self, popdir):
        self.popdir = popdir
        self.load_popfiles

    def load_popfiles(self):
        base = os.path.basename(self.popdir)
        self.population = pd.DataFrame.from_csv(
                os.path.join(self.popdir, '%s_synth_people.txt' % base)
        self.population.reset_index(inplace=True)
        self.population['person'] = self.population.index
        self.households = pd.DataFrame.from_csv(
                os.path.join(self.popdir, '%s_synth_households.txt' % base)
        self.households.reset_index()
        self.households['stcotr'] = (self.households.stcotrbg/10).astype(np.int64)
        apollo_locations = pd.DataFrame.from_csv('ApolloLocationCode.to.FIPSstcotr.csv')
        apollo_locations.reset_index(level=0, inplace=True)
        self.households = pd.merge(self.households, apollo_locations,
            on='stcotr', how='inner', suffixes=('','_'))
        self.workplaces = pd.DataFrame.from_csv(
                os.path.join(self.popdir, '%s_workplaces.txt' % base)
        self.schools = pd.DataFrame.from_csv(
                os.path.join(self.popdir, '%s_schools.txt' % base)

    




import pandas as pd
import numpy as np
import ujson, yaml, time, bz2, gzip, os, re
try:
    import lzma
except ImportError as e:
    import backports.lzma as lzma
from collections import OrderedDict, defaultdict
#import pyximport
#pyximport.install(reload_support=True)
#import cyprinev.count_events as count_events
import count_events
import logging

logging.basicConfig(level=logging.DEBUG, format='[%(name)s] %(asctime)s %(message)s')

log = logging.getLogger(__name__)

class Timer():
    def __init__(self):
        self.tic = time.time()
    def __call__(self):
        toc = time.time()
        t = '%0.1f' % (toc - self.tic)
        self.tic = toc
        return t

def AutoDetectFile(filename):
    filetypes = [
            ('bz2', bz2.BZ2File, '\x42\x5a\x68'),
            ('gzip', gzip.GzipFile, '\x1f\x8b\x08'),
            ('lzma', lzma.LZMAFile, '\xfd7zXZ\x00')]
    for typename, fileopen, magic in filetypes:
        try:
            with open(filename, 'r') as f:
                if f.read(len(magic)) != magic:
                    raise Exception()
            f = fileopen(filename, 'r')
            log.info('Opened %s as %s' % (filename, typename))
            return f
        except:
            log.debug('Unsuccessfully tried to open as %s' % typename)
    log.info('Opening as plain text file')
    f = open(filename, 'r')
    return f

class OutputCollection(object):

    @property
    def state_dict(self):
        return dict(N = 'number of individuals', S = 'susceptible',
                E = 'exposed', I = 'infectious', Y = 'symptomatic',
                R = 'recovered', IS = 'infectious and symptomatic')
        
    @property
    def population_dict(self):
        return dict(race = 'race', age = 'age', gender = 'sex')

    @property
    def household_dict(self):
        return dict(income = 'hh_income', stcotrbg = 'stcotrbg',
                stcotr = 'stcotr', location = 'apollo_location_code')

    @property
    def default_config(self):
        d = os.path.dirname(os.path.abspath(__file__))
        f = os.path.join(d, 'default_group_config.yaml')
        return open(f, 'r')

    @property
    def event_map(self):
        return OrderedDict([('exposed',0), ('infectious',1), ('symptomatic',2),
            ('recovered',3), ('susceptible',4), ('vaccine',5), ('vaccine_day',6)])

    @property
    def state_map(self):
        return OrderedDict([('N_i',0),('N_p',1),('S_i',2),('S_p',3),('E_i',4),
            ('E_p',5),('I_i',6),('I_p',7),('Y_i',8),('Y_p',9),('R_i',10),
            ('R_p',11),('IS_i',12),('IS_p',13),('V_i',14),('V_p',15)])

    def __init__(self, popdir):
        log.debug('read default group config: %s' % [
            str(yaml.load(self.default_config))])
        self.popdir = popdir
        self.load_popfiles()

    def load_popfiles(self):
        base = os.path.basename(self.popdir)
        timer = Timer()
        self.population = pd.read_csv(
                os.path.join(self.popdir, '%s_synth_people.txt' % base),
                low_memory=False)
        self.population.reset_index(inplace=True)
        self.population['person'] = self.population.index
        log.info('read population in %s seconds' % timer())
        self.households = pd.read_csv(
                os.path.join(self.popdir, '%s_synth_households.txt' % base))
        self.households.reset_index()
        self.households['stcotr'] = (self.households.stcotrbg/10).astype(np.int64)
        apollo_locations = pd.read_csv('ApolloLocationCode.to.FIPSstcotr.csv')
        apollo_locations.reset_index(level=0, inplace=True)
        self.households = pd.merge(self.households, apollo_locations,
            on='stcotr', how='inner', suffixes=('','_'))
        log.info('read households in %s seconds' % timer())
        self.workplaces = pd.read_csv(
                os.path.join(self.popdir, '%s_workplaces.txt' % base))
        log.info('read workplaces in %s seconds' % timer())
        self.schools = pd.read_csv(
                os.path.join(self.popdir, '%s_schools.txt' % base))
        log.info('read schools in %s seconds' % timer())

    def query_population(self, groupby_attributes): 
    
        _rev_population_dict = {self.population_dict[x]:x for x in \
                groupby_attributes if x in self.population_dict}
        
        _rev_population_dict.update({'person':'person'})
        
        _rev_household_dict = {self.household_dict[x]:x for x in \
                groupby_attributes if x in self.household_dict}

        _population = pd.merge(self.population[_rev_population_dict.keys() + ['hh_id']],
                               self.households[_rev_household_dict.keys() + ['hh_id']],
                               on='hh_id', suffixes=('', '.h'), how='inner',
                               copy=True)[_rev_population_dict.keys() + \
                                          _rev_household_dict.keys()]
        
        _population.rename(columns=_rev_population_dict, inplace=True, copy=False)
        _population.rename(columns=_rev_household_dict, inplace=True, copy=False)

        for k in groupby_attributes:
            if k not in list(_population.columns):
                raise Exception('Unable to group by key: %s' % k)
        
        return _population
        
    def apply_count_events(self, events, group_by_keys):
        timer = Timer() 
        DTYPE = np.uint32
        NA = DTYPE(-1)
 
        d_query_pop = self.query_population(group_by_keys)

        log.info('Extracted groups from population data in %s seconds' % timer())
       
        d = pd.merge(events['infection'], d_query_pop,
                     on='person', how='right', suffixes=('','_')
                    ).sort_values(group_by_keys).reset_index(drop=True)

        if 'vaccination' not in events:
            d_vacc = pd.DataFrame(dict(person=[], vaccine=[], vaccine_day=[]))
        else:
            d_vacc = events['vaccination']

        d = pd.merge(d, d_vacc, on='person', how='left')

        d = d[self.event_map.keys() + group_by_keys]
       
        d[self.event_map.keys()] = d[self.event_map.keys()].fillna(NA).apply(
                lambda x: x.astype(DTYPE), axis=0)

        n_days = d.recovered[d.recovered != NA].max() + 1

        colnames = [k for k,v in sorted(self.state_map.items(),
                                        key=lambda x: x[1])]
        def convert_counts_array(g):
            a = count_events.get_counts_from_group(g.values.astype(np.uint32),
                                                   np.uint32(n_days),
                                                   self.event_map, self.state_map)
            df = pd.DataFrame(np.asarray(a), columns=colnames)
            df.index.name = 'day'
            return df

        log.info('Merged events with population data in %s seconds' % timer())

        grouped_counts = d.groupby(group_by_keys).apply(convert_counts_array)

        log.info('Tabulated grouped event counts in %s seconds' % timer())
        
        return grouped_counts 

    def read_event_report(self, filename):
        output_lists = defaultdict(list)
        with AutoDetectFile(filename) as f:
            for line in f:
                j = ujson.loads(line)
                output_lists[j.pop('event')].append(j)
        return {k:pd.DataFrame(v) for k,v in output_lists.iteritems()}

    def count_events(self, reportfiles, outfile, groupconfig=None):
        hdf_outfile_name = '%s.h5' % outfile
        hdf = pd.HDFStore(path=hdf_outfile_name, mode='w', complevel=9, complib='bzip2')
        for f in reportfiles:
            k = os.path.basename(f)
            k = re.sub(r'[-.+ ]', '_', k)
            events = self.read_event_report(f)
            log.info('Read %s events from %s' % (', '.join(events.keys()), f))
            counts = self.apply_count_events(events, ['age', 'gender', 'race', 'location'])
            hdf.append(k, counts)
            hdf.put('%s/paramters', events['parametrs'])
        hdf.close

            































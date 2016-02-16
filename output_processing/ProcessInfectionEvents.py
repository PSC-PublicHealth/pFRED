
# coding: utf-8

# In[290]:

#%load_ext Cython
import pandas as pd
import numpy as np
import json
from joblib import Parallel, delayed
from joblib.pool import has_shareable_memory
import multiprocessing
import itertools
import time
import lz4
import pandas as pd
import numpy as np
from collections import OrderedDict
import time
import pyximport
pyximport.install(reload_support=True)
import cyprinev.count_events as count_events


# In[2]:

def read_infection_events_to_data_frame(filename='infection.events.json'):
    def read_infection_events():
        with open(filename, 'rb') as f:
            for line in f:
                yield(json.loads(line))
    infections = pd.DataFrame.from_dict(read_infection_events())
    infections.loc[infections.symptomatic < 1, 'symptomatic'] = None
    infections.person = infections.person.astype(np.int64)
    return(infections)


# In[3]:

#%%timeit -n1 -r1
infections = read_infection_events_to_data_frame()


# In[4]:

#%%timeit -n1 -r1
population_original = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_people.txt')
households_original = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_households.txt')
workplaces = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_workplaces.txt')
schools = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_schools.txt')


# In[5]:

population = population_original.copy()
population = population.reset_index()
population['person'] = population.index
population['age_group'] = pd.cut(population.age, bins=range(0,120,10), include_lowest=True, right=False)
#population['age_group'] = pd.cut(population.age, bins=[0,2,18,65,120], include_lowest=True, right=False)

apollo_locations = pd.DataFrame.from_csv('ApolloLocationCode.to.FIPSstcotr.csv')
apollo_locations.reset_index(level=0, inplace=True)

households = households_original.copy().reset_index()
households['stcotr'] = (households.stcotrbg/10).astype(np.int64)
households = pd.merge(households, apollo_locations, on='stcotr', how='inner', suffixes=('','_'), copy=True)


# In[306]:

state_dict = dict(
    N = 'number of individuals',
    S = 'susceptible', E = 'exposed', I = 'infectious',
    Y = 'symptomatic', R = 'recovered', IS = 'infectious and symptomatic'
)
population_dict = dict(
    # FRED currently just numbers persons sequentially as they are read from the synthetic population file
    # rather than using the unique p_id provided in the synth population.  This should be changed, but until it
    # is, we can't use the p_id.  Instead, see cell above for 'person' sequential id column.
    #person = 'p_id', 
    race = 'race',
    age = 'age',
    gender = 'sex'
)
household_dict = dict(
    income = 'hh_income',
    stcotrbg = 'stcotrbg',
    stcotr = 'stcotr',
    location = 'apollo_location_code'
)


# In[307]:

def query_population(population, households, groupby_attributes): 

    _rev_population_dict = {population_dict[x]:x for x in groupby_attributes if x in population_dict}
    _rev_population_dict.update({'person':'person'})
    
    _rev_household_dict = {household_dict[x]:x for x in groupby_attributes if x in household_dict}

    _population = pd.merge(population[_rev_population_dict.keys() + ['hh_id']],
                           households[_rev_household_dict.keys() + ['hh_id']],
                           on='hh_id', suffixes=('', '.h'), how='inner',
                           copy=True)[_rev_population_dict.keys() + _rev_household_dict.keys()]
    
    _population.rename(columns=_rev_population_dict, inplace=True, copy=False)
    _population.rename(columns=_rev_household_dict, inplace=True, copy=False)

    for k in groupby_attributes:
        if k not in list(_population.columns):
            raise Exception('Unable to group by key: %s' % k)
    
    return _population


# In[339]:

reload(count_events)

def apply_count_events(population, households, infections,
                       group_by_keys=['age','race','location','gender']):
    
    DTYPE = np.uint32
    NA = DTYPE(-1)
    
    d = pd.merge(infections.fillna(NA).apply(lambda x: x.astype(DTYPE), axis=0),
                 query_population(population, households, group_by_keys),
                 on='person', how='inner', suffixes=('','_')
                ).sort_values(group_by_keys).reset_index(drop=True)
    
    event_map = OrderedDict(
        exposed = d.columns.get_loc('exposed'),
        infectious = d.columns.get_loc('infectious'),
        symptomatic = d.columns.get_loc('symptomatic'),
        recovered = d.columns.get_loc('recovered'),
        susceptible = d.columns.get_loc('susceptible')
    )

    state_map = OrderedDict(
        N_i=0,S_i=2,E_i=4,I_i=6,Y_i=8,R_i=10,IS_i=12,
        N_p=1,S_p=3,E_p=5,I_p=7,Y_p=9,R_p=11,IS_p=13
    )

    n_days = d.recovered[d.recovered != NA].max() + 1

    colnames = [k for k,v in sorted(state_map.items(),
                                    key=lambda x: x[1])]
    def convert_counts_array(g):
        a = count_events.get_counts_from_group(g.values.astype(np.uint32),
                                               np.uint32(n_days),
                                               event_map, state_map)
        df = pd.DataFrame(np.asarray(a), columns=colnames)
        df.index.name = 'day'
        return df

    return d.groupby(group_by_keys).apply(convert_counts_array)
    


# In[341]:

tic = time.time()
test = apply_count_events(population, households, infections, [
        'age',
        'race',
        'location',
        'gender'
    ])
print(time.time() - tic)


# In[280]:

def convert_columns(r):
    # conversion of columns done as side-effect; data frame passed by ref
    if ('gender' in r.keys() and 
        not all([a==b for (a,b) in zip(sorted(r.gender.unique()),
                                       sorted(['M','F']))])):
        r.gender = pd.cut(r.gender,bins=2,labels=['M','F'])
    return r


# In[320]:

convert_columns(test.reset_index()).to_hdf(
    'output.hdf5', key='AlleghenyCounty_42003_100_Days',
    mode='w', format='t', complevel=9, complib='bzip2')


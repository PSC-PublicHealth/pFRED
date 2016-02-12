
# coding: utf-8

# In[ ]:

import pandas as pd
import numpy as np
import json
#try: import simplejson as json
#except ImportError: import json
from joblib import Parallel, delayed
from joblib.pool import has_shareable_memory
import multiprocessing
import itertools
import time
import lz4


# In[ ]:

def read_infection_events_to_data_frame(filename='infection.events.json'):
    def read_infection_events():
        with open(filename, 'rb') as f:
            for line in f:
                yield(json.loads(line))
    infections = pd.DataFrame.from_dict(read_infection_events())
    infections.loc[infections.symptomatic < 1, 'symptomatic'] = None
    infections.person = infections.person.astype(np.int64)
    return(infections)


# In[ ]:

#%%timeit -n1 -r1
infections = read_infection_events_to_data_frame()


# In[ ]:

#%%timeit -n1 -r1
population_original = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_people.txt')
households_original = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_households.txt')
workplaces = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_workplaces.txt')
schools = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_schools.txt')


# In[ ]:

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


# In[ ]:

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
    age_group = 'age_group',
    age = 'age',
    gender = 'sex'
)
household_dict = dict(
    income = 'hh_income',
    #location = 'stcotrbg',
    #location = 'stcotr',
    location = 'apollo_location_code'
)


# In[ ]:

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


# In[ ]:

def query_infections(group_data_frame, times, incidence=['N','S','E','I','Y','R','IS'],
                     prevalence=['N','S','E','I','Y','R','IS'], grouping_keys={}):
    
    local_infections = pd.merge(infections, group_data_frame,
                                on='person', how='inner', suffixes=('','_'), copy=True)

    total_local_persons = len(group_data_frame.index)
    
    def get_incidence(local_infections, state, day):
        if state == 'IS':
            return len(local_infections[(day == local_infections.infectious) &                                         (day == local_infections.symptomatic)].index)
        elif state == 'N':
            return 0
        else:
            s = state_dict[state]
            return len(local_infections[local_infections[s]==day].index)

    def get_prevalence(local_infections, states, day):
        if states is not None and isinstance(states, list) and len(states) > 0:
            mask = {'NOT_S': (day >= local_infections.exposed) &                              (day < local_infections.susceptible)}
            if set(['E','I','Y','R','IS']).intersection(set(states)):
                mask['E'] = (mask['NOT_S'] & (day < local_infections.infectious))
            if 'I' in states:
                mask['I'] = (mask['NOT_S'] & (day >= local_infections.infectious)) &                             (day < local_infections.recovered)
            if 'Y' in states:
                mask['Y'] = (mask['NOT_S'] & (day >= local_infections.symptomatic)) &                             (day < local_infections.recovered)
            if 'R' in states:
                mask['R'] = (mask['NOT_S'] & (day >= local_infections.recovered)) &                             (day < local_infections.susceptible)
            if 'IS' in states:
                mask['IS'] = mask['I'] & mask['Y']
           
        for state in states:
            if state is None:
                raise Exception('No state specified!')
            elif state == 'S':
                yield (state, total_local_persons - len(local_infections[mask['NOT_S']].index))
            elif state == 'N':
                yield (state, total_local_persons)
            else:
                try:
                    if total_local_persons == len(local_infections[mask['NOT_S']].index):
                        yield (state, 0)
                    else:
                        yield (state, len(local_infections[mask[state]].index))
                except:
                    raise Exception('Unknown prevalence state requested %s!' % state)

    rows = []
    for t in times:
        row = dict(day = t)
        row.update({k + '_i': get_incidence(local_infections, k, t) for k in incidence})
        row.update({k + '_p': v for k,v in get_prevalence(local_infections, prevalence, t)})
        if grouping_keys:
            row.update(grouping_keys)
        rows.append(row)
    
    return lz4.dumps(json.dumps(rows))


# In[ ]:

def apply_query_infections(population, households, times,
                           incidence=['N','S','E','I','Y','R','IS'],
                           prevalence=['N','S','E','I','Y','R','IS'],
                           group_by_keys=['age','race'],
                           parallel=True):
    
    pop = query_population(population, households, group_by_keys)
    grouped_persons = pop.groupby(group_by_keys)
    grouping_keys = grouped_persons.grouper.names

    if parallel:
        n_jobs = multiprocessing.cpu_count()
        
        # NOTE: The "mmap_mode='c'" argument (copy-on-write) is necessary for
        # processing groupby (which are MemeoryViews) even if you're not 
        # writing to them! This is crazy!
        result_list = Parallel(n_jobs=n_jobs, mmap_mode='c')(
            delayed(query_infections)(      
                group_data_frame,
                times, incidence, prevalence,
                {k:v for k,v in zip(
                        grouping_keys,
                        grouping_values if isinstance(grouping_values, tuple) \
                                        else (grouping_values,))
                }) for grouping_values, group_data_frame in grouped_persons)
    else:
        result_list = [query_infections(      
                group_data_frame,
                times, incidence, prevalence,
                {k:v for k,v in zip(
                        grouping_keys,
                        grouping_values if isinstance(grouping_values, tuple) \
                                        else (grouping_values,))
                }) for grouping_values, group_data_frame in grouped_persons]
    
    return pd.DataFrame([row_dict for row_dict in itertools.chain(
                *[json.loads(lz4.loads(result)) for result in result_list])])

# check this out for a little background:
# http://stackoverflow.com/questions/26187759/parallelize-apply-after-pandas-groupby


# In[ ]:

times = range(100)
tic = time.time()
r = apply_query_infections(population, households, times=times,
                           group_by_keys=['age_group','gender'],
                           parallel=True)
print(time.time() - tic)


# In[ ]:

def convert_columns(r):
    # conversion of columns done as side-effect; data frame passed by ref
    if ('gender' in r.keys() and 
        not all([a==b for (a,b) in zip(sorted(r.gender.unique()),
                                       sorted(['M','F']))])):
        r.gender = pd.cut(r.gender,bins=2,labels=['M','F'])
    return r


# In[ ]:

convert_columns(r).to_hdf('output.hdf5', key='AlleghenyCounty_42003_100_Days',
                          mode='w', format='t', complevel=9, complib='bzip2')


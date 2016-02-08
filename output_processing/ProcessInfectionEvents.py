
# coding: utf-8

# In[ ]:

import pandas as pd
import numpy as np
import json
from joblib import Parallel, delayed
import multiprocessing
import itertools
import time


# In[ ]:

def read_infection_events_to_data_frame(filename='infection.events.json'):
    def read_infection_events():
        with open(filename, 'rb') as f:
            for line in f:
                yield(json.loads(line))
    infections = pd.DataFrame.from_dict(read_infection_events())
    infections.loc[infections.symptomatic < 1, 'symptomatic'] = None
    return(infections)


# In[ ]:

#%%timeit -n1 -r1
infections = read_infection_events_to_data_frame()


# In[264]:

#%%timeit -n1 -r1
population = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_people.txt')
households_original = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_synth_households.txt')
workplaces = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_workplaces.txt')
schools = pd.DataFrame.from_csv('../populations/2005_2009_ver2_42003/2005_2009_ver2_42003_schools.txt')


# In[265]:

households = households_original.reset_index(level=0)
households['stcotr'] = (households.stcotrbg/10).astype(np.int64)
#population['stcotr'] = (population.stcotrbg/10).astype(np.int64)

apollo_locations = pd.DataFrame.from_csv('ApolloLocationCode.to.FIPSstcotr.csv')
apollo_locations.reset_index(level=0, inplace=True)

households = pd.merge(households, apollo_locations, on='stcotr', how='inner', suffixes=('','_'), copy=True)
#population = pd.merge(population, apollo_locations, on='stcotr', how='inner', suffixes=('','_'), copy=True)


# In[266]:

state_dict = dict(
    S = 'susceptible', E = 'exposed', I = 'infectious',
    Y = 'symptomatic', R = 'recovered', IS = 'infectious and symptomatic'
)
population_dict = dict(
    person = 'p_id',
    race = 'race',
    age = 'age',
    sex = 'sex'
)
household_dict = dict(
    income = 'hh_income',
    location = 'apollo_location_code'
)


# In[267]:

def query_population(population, households,
                     population_attributes=['age','race','sex'],
                     household_attributes=['income','location']):
    _population_attributes = [population_dict[x] for x in population_attributes] 
    _household_attributes = [household_dict[x] for x in household_attributes]
    #population = population[_population_attributes+['hh_id',]].join(households[_household_attributes],
    #               on='hh_id', lsuffix='', rsuffix='.h',
    #               how='left')[list(set(_population_attributes).union(_household_attributes))].reset_index(level=0)
    population = pd.merge(population.reset_index(level=0)[_population_attributes+['p_id','hh_id',]],
                          households.reset_index(level=0)[_household_attributes+['hh_id']],
                          on='hh_id', suffixes=('', '.h'), how='left',
                          copy=True)[list(set(_population_attributes + ['p_id']
                                             ).union(_household_attributes))]
    population.rename(columns = {v:k for k,v in dict(population_dict.items() + household_dict.items()).iteritems()},
                      inplace = True)
    return population


# In[268]:

def query_infections(infections, times, incidence=['S','E','I','Y','R','IS'],
                     prevalence=['S','E','I','Y','R','IS'], grouping_keys={}):
    # NOTE: Can't do a join inside a Parallel for some reason, so must join infections and persons before passing!!!
    total_susceptible = len(infections.person.unique())
    
    def get_incidence(infections, state, day):
        if state == 'IS':
            return len(infections[(day == infections.infectious) & (day == infections.symptomatic)].index)
        else:
            s = state_dict[state]
            return len(infections[infections[s]==day].index)

    def get_prevalence(infections, states, day):
        if states is not None and isinstance(states, list) and len(states) > 0:
            mask = {'NOT_S': (day >= infections.exposed) & (day < infections.susceptible)}
            if set(['E','I','Y','R','IS']).intersection(set(states)):
                mask['E'] = (mask['NOT_S'] & (day < infections.infectious))
            if 'I' in states:
                mask['I'] = (mask['NOT_S'] & (day >= infections.infectious)) & (day < infections.recovered)
            if 'Y' in states:
                mask['Y'] = (mask['NOT_S'] & (day >= infections.symptomatic)) & (day < infections.recovered)
            if 'R' in states:
                mask['R'] = (mask['NOT_S'] & (day >= infections.recovered)) & (day < infections.susceptible)
            if 'IS' in states:
                mask['IS'] = mask['I'] & mask['Y']
           
        for state in states:
            if state is None:
                raise Exception('No state specified!')
            elif state == 'S':
                yield (state, total_susceptible - len(infections[mask['NOT_S']].index))
            else:
                try:
                    yield (state, len(infections[mask[state]].index))
                except:
                    raise Exception('Unknown prevalence state requested %s!' % state)

    rows = []
    for t in times:
        row = dict(day = t)
        row.update({k + '_i': get_incidence(infections, k, t) for k in incidence})
        row.update({k + '_p': v for k,v in get_prevalence(infections, prevalence, t)})
        if grouping_keys:
            row.update(grouping_keys)
        rows.append(row)
    
    return pd.DataFrame(rows)


# In[269]:

def parallel_apply_query_infections(population, households, infections, times,
                                    incidence=['S','E','I','Y','R','IS'],
                                    prevalence=['S','E','I','Y','R','IS'],
                                    group_by_keys=['age','race']):
    
    n_jobs = multiprocessing.cpu_count()
    
    grouped_persons = query_population(population, households).groupby(group_by_keys)
    grouping_keys = grouped_persons.grouper.names
    
    result_list = Parallel(n_jobs=n_jobs)(delayed(query_infections)(
            #infections.join(group_data_frame, on='person', how='inner', lsuffix='', rsuffix='_'),
            pd.merge(infections, group_data_frame, on='person', how='inner', suffixes=('','_'), copy=False),
            times, incidence, prevalence,
            {k:v for k,v in zip(grouping_keys,
                                grouping_values if isinstance(grouping_values, tuple) \
                                                else (grouping_values,))
            }) for grouping_values, group_data_frame in grouped_persons)
    
    return pd.concat(result_list)
# check this out for a little background:
# http://stackoverflow.com/questions/26187759/parallelize-apply-after-pandas-groupby


# In[271]:

tic = time.time()

r = parallel_apply_query_infections(population, households, infections,
                                    times=range(10),
                                    #incidence=['E'], prevalence=['I'],
                                    group_by_keys=['location','age','sex'])

toc = time.time()

print(toc-tic)
print(len(r))
print(r.head())


# In[ ]:

len(r)


# In[ ]:

r.to_csv('test_trajectories.csv',header=True,index=False)


# In[ ]:

query_population(population, households)


# In[263]:

households.head()


# In[272]:

query_population(population, households)


# In[ ]:




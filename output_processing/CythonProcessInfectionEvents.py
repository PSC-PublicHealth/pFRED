
# coding: utf-8

# In[60]:

get_ipython().magic(u'reload_ext Cython')
import pandas as pd
import numpy as np
from scipy.sparse import csr_matrix
from collections import OrderedDict
import time
#import numba


# In[3]:

# NOTE: Inital setting for NA; may change later when coerced to DTYPE!
NA = -1
DTYPE = np.uint32
d_full = pd.read_csv('/tmp/test.csv')
d_full.fillna(NA, inplace=True)
d_full.ix[d_full.gender != NA,'gender'] -= 1
d_full = d_full.apply(lambda x: x.astype(DTYPE), axis=0)
# NOTE: NA may change when coerced to DTYPE!
NA = DTYPE(NA)


# In[4]:

d_full.head()


# In[65]:

d = d_full

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

grouping_keys = ['age','gender']
group_map = OrderedDict([(k,d.columns.get_loc(k)) for k in grouping_keys])
group_dims_sorted_indexes = [i for k,i in group_map.iteritems()]
group_dims_sizes = [int(d[k][d[k]!=NA].max()) + 1 for k in group_map.keys()]

dim_states_size = len(state_map)
dim_days_size = int(d.recovered[d.recovered != NA].max()) + 1

a = np.zeros(group_dims_sizes+[dim_states_size,dim_days_size], dtype=DTYPE)
a.shape


# In[66]:

get_ipython().run_cell_magic(u'cython', u'', u"from __main__ import NA, DTYPE\nfrom __main__ import state_map, event_map, group_map\nfrom __main__ import dim_days_size, group_dims_sorted_indexes\nfrom __main__ import a\n\nlast_day = DTYPE(dim_days_size - 1)\nnum_days = DTYPE(dim_days_size)\n\nexposed = event_map['exposed']\ninfectious = event_map['infectious']\nsymptomatic = event_map['symptomatic']\nrecovered = event_map['recovered']\nsusceptible = event_map['susceptible']\n\nN_i = state_map['N_i']\nS_i = state_map['S_i']\nE_i = state_map['E_i']\nI_i = state_map['I_i']\nY_i = state_map['Y_i']\nR_i = state_map['R_i']\nIS_i = state_map['IS_i']\n\nN_p = state_map['N_p']\nS_p = state_map['S_p']\nE_p = state_map['E_p']\nI_p = state_map['I_p']\nY_p = state_map['Y_p']\nR_p = state_map['R_p']\nIS_p = state_map['IS_p']\n\ndef get_counts(r):\n    group_indexes = []\n    for g in group_dims_sorted_indexes:\n        if r[g] == NA:\n            return False\n        else:\n            group_indexes.append([r[g]])\n\n    # always increment N_p (if groupings are not null)\n    a[group_indexes + [[N_p]]] += 1\n\n    if r[exposed] == NA:\n        a[group_indexes + [[S_p]]] += 1\n    else:\n        a[group_indexes + [[E_i]] + [[r[exposed]]]] += 1\n        a[group_indexes + [[E_p]] + [range(r[exposed], r[infectious])]] += 1\n        \n        a[group_indexes + [[S_p]] + [range(0, r[exposed])]] += 1\n        if r[susceptible] != NA:\n            if r[susceptible] < last_day:\n                a[group_indexes + [[S_p]] + [range(r[susceptible], num_days)]] += 1\n            elif r[susceptible] == last_day:\n                a[group_indexes + [[S_p]] + [r[susceptible]]] += 1\n                    \n        if r[infectious] != NA:\n            a[group_indexes + [[I_i]] + [[r[infectious]]]] += 1\n            a[group_indexes + [[I_p]] + [range(r[infectious], r[recovered])]] += 1\n            \n        if r[symptomatic] != NA:\n            a[group_indexes + [[Y_i]] + [[r[symptomatic]]]] += 1\n            a[group_indexes + [[Y_p]] + [range(r[symptomatic], r[recovered])]] += 1\n            # NOTE: by default all symptomatics are infectious; this is a shortcut \n            a[group_indexes + [[IS_i]] + [[r[symptomatic]]]] += 1\n            a[group_indexes + [[IS_i]] + [range(r[symptomatic], r[recovered])]] += 1\n           \n        if r[recovered] != NA:\n            a[group_indexes + [[R_i]] + [[r[recovered]]]] += 1\n            if r[recovered] == last_day:\n                a[group_indexes + [[R_p]] + [[r[recovered]]]] += 1\n            else:\n                assert(r[recovered] < last_day)\n                a[group_indexes + [[R_p]] + [range(r[recovered], num_days)]] += 1\n                \n    return True")


# In[67]:

#%prun -l 2 d.apply(get_counts, axis=1, raw=True)
#test2=d.apply(get_counts, axis=1, raw=True)


# In[68]:

tic = time.time()
a.fill(DTYPE(0))
d.sample(frac=0.5).apply(get_counts, axis=1, raw=True)
b=a.copy()
a.fill(DTYPE(0))
d.sample(frac=0.5).apply(get_counts, axis=1, raw=True)
c=a+b
print time.time()-tic


# In[ ]:




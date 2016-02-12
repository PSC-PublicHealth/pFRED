
# coding: utf-8

# In[1]:

#%reload_ext Cython
import pandas as pd
import numpy as np
#from scipy.sparse import csr_matrix
from collections import OrderedDict
import time
#import numba


# In[2]:

# NOTE: Inital setting for NA; may change later when coerced to DTYPE!
NA = -1
DTYPE = np.uint32
d_full = pd.read_csv('/tmp/test.csv')
d_full.fillna(NA, inplace=True)
d_full.ix[d_full.gender != NA,'gender'] -= 1
d_full = d_full.apply(lambda x: x.astype(DTYPE), axis=0)
# NOTE: NA may change when coerced to DTYPE!
NA = DTYPE(NA)


# In[3]:

d_full.head()


# In[4]:

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


# ```python
# %%cython
# from __main__ import NA, DTYPE
# from __main__ import state_map, event_map, group_map
# from __main__ import dim_days_size, group_dims_sorted_indexes
# from __main__ import a
# 
# last_day = DTYPE(dim_days_size - 1)
# num_days = DTYPE(dim_days_size)
# 
# exposed = event_map['exposed']
# infectious = event_map['infectious']
# symptomatic = event_map['symptomatic']
# recovered = event_map['recovered']
# susceptible = event_map['susceptible']
# 
# N_i = state_map['N_i']
# S_i = state_map['S_i']
# E_i = state_map['E_i']
# I_i = state_map['I_i']
# Y_i = state_map['Y_i']
# R_i = state_map['R_i']
# IS_i = state_map['IS_i']
# 
# N_p = state_map['N_p']
# S_p = state_map['S_p']
# E_p = state_map['E_p']
# I_p = state_map['I_p']
# Y_p = state_map['Y_p']
# R_p = state_map['R_p']
# IS_p = state_map['IS_p']
# 
# def get_counts(r):
#     group_indexes = []
#     for g in group_dims_sorted_indexes:
#         if r[g] == NA:
#             return False
#         else:
#             group_indexes.append([r[g]])
# 
#     # always increment N_p (if groupings are not null)
#     a[group_indexes + [[N_p]]] += 1
# 
#     if r[exposed] == NA:
#         a[group_indexes + [[S_p]]] += 1
#     else:
#         a[group_indexes + [[E_i]] + [[r[exposed]]]] += 1
#         a[group_indexes + [[E_p]] + [range(r[exposed], r[infectious])]] += 1
#         
#         a[group_indexes + [[S_p]] + [range(0, r[exposed])]] += 1
#         if r[susceptible] != NA:
#             if r[susceptible] < last_day:
#                 a[group_indexes + [[S_p]] + [range(r[susceptible], num_days)]] += 1
#             elif r[susceptible] == last_day:
#                 a[group_indexes + [[S_p]] + [r[susceptible]]] += 1
#                     
#         if r[infectious] != NA:
#             a[group_indexes + [[I_i]] + [[r[infectious]]]] += 1
#             a[group_indexes + [[I_p]] + [range(r[infectious], r[recovered])]] += 1
#             
#         if r[symptomatic] != NA:
#             a[group_indexes + [[Y_i]] + [[r[symptomatic]]]] += 1
#             a[group_indexes + [[Y_p]] + [range(r[symptomatic], r[recovered])]] += 1
#             # NOTE: by default all symptomatics are infectious; this is a shortcut 
#             a[group_indexes + [[IS_i]] + [[r[symptomatic]]]] += 1
#             a[group_indexes + [[IS_i]] + [range(r[symptomatic], r[recovered])]] += 1
#            
#         if r[recovered] != NA:
#             a[group_indexes + [[R_i]] + [[r[recovered]]]] += 1
#             if r[recovered] == last_day:
#                 a[group_indexes + [[R_p]] + [[r[recovered]]]] += 1
#             else:
#                 assert(r[recovered] < last_day)
#                 a[group_indexes + [[R_p]] + [range(r[recovered], num_days)]] += 1
#                 
#     return True
# ```

# In[13]:

get_ipython().magic(u'prun -l 2 d.apply(get_counts, axis=1, raw=True)')
#test2=d.apply(get_counts, axis=1, raw=True)


# The following cells reproduce the important bits of the cython cell magic.
# For guidance look here:
# - https://raw.githubusercontent.com/studer/ipython/master/IPython/extensions/cythonmagic.py

# In[9]:

from distutils.core import Distribution, Extension
from distutils.command.build_ext import build_ext
from Cython.Build import cythonize
import os
#os.environ["CC"] = "gcc"
#os.environ["CXX"] = "gcc"
#os.environ["CPP"] = "gcc"
dist = Distribution()
build_extension = build_ext(dist)
build_extension.finalize_options()


# In[10]:

opts = dict(
    #quiet=quiet,
    #annotate = args.annotate,
    force = True,
)
cyprinev_extension = Extension(
    name = 'count_events',
    sources = ['cyprinev/count_events.pyx'],
    include_dirs = [np.get_include()],
    language = 'c',
    #library_dirs = args.library_dirs,
    #extra_compile_args=['-fopenmp'],
    #extra_link_args=['-fopenmp']
    #libraries = args.lib,
    #language = 'c++' if args.cplus else 'c',
)
build_extension.extensions = cythonize([cyprinev_extension])
build_extension.build_temp = 'cyprinev/build_temp'
build_extension.build_lib  = 'cyprinev'


# In[11]:

build_extension.run()


# In[33]:

#from cyprinev.count_events import get_counts
reload(count_events)


# In[25]:

g = d.groupby(['age','gender'])


# In[12]:

dist.find_config_files()


# In[14]:

from distutils import sysconfig
sysconfig.get_config_var('LDSHARED')


# In[ ]:




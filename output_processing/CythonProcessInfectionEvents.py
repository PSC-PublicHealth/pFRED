
# coding: utf-8

# In[1]:

#%reload_ext Cython
import pandas as pd
import numpy as np
#from scipy.sparse import csr_matrix
from collections import OrderedDict
import time
#import numba

# NOTE! need to do this to be able to reload modules!!!!
import pyximport
pyximport.install(reload_support=True)


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

grouping_keys = ['age','gender']
d = d_full.sort_values(grouping_keys).reset_index(drop=True)

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

group_map = OrderedDict([(k,d.columns.get_loc(k)) for k in grouping_keys])
group_dims_sorted_indexes = [i for k,i in group_map.iteritems()]
group_dims_sizes = [int(d[k][d[k]!=NA].max()) + 1 for k in group_map.keys()]

dim_states_size = len(state_map)
dim_days_size = int(d.recovered[d.recovered != NA].max()) + 1

a = np.zeros(group_dims_sizes+[dim_states_size,dim_days_size], dtype=DTYPE)
a.shape


# In[ ]:

#%prun -l 2 d.apply(get_counts, axis=1, raw=True)
#test2=d.apply(get_counts, axis=1, raw=True)


# The following cells reproduce the important bits of the cython cell magic.
# For guidance look here:
# - https://raw.githubusercontent.com/studer/ipython/master/IPython/extensions/cythonmagic.py

# In[109]:

from distutils.core import Distribution, Extension
from distutils.command.build_ext import build_ext
from Cython.Build import cythonize
import os
os.environ["CC"] = "/usr/local/gcc483/bin/gcc"
os.environ["CXX"] = "/usr/local/gcc483/bin/g++"
os.environ["OMP_NUM_THREADS"] = "10"
dist = Distribution()
build_extension = build_ext(dist)
#build_extension.finalize_options()
opts = dict(
    #quiet=quiet,
    #annotate = args.annotate,
    force = True,
)
cyprinev_extension = Extension(
    name = 'count_events',
    sources = ['cyprinev/count_events.pyx'],
    include_dirs = [np.get_include()],
    language = 'c++',
    #library_dirs = ['/usr/local/lib/gcc/5','/usr/local/lib'],
    extra_compile_args=['-fopenmp'],
    extra_link_args=['-fopenmp']
    #libraries = args.lib,
    #language = 'c++' if args.cplus else 'c',
)
build_extension.extensions = cythonize([cyprinev_extension], force=True)
build_extension.build_temp = 'cyprinev/build_temp'
build_extension.build_lib  = 'cyprinev'
build_extension.finalize_options()
build_extension.run()
#import cyprinev.count_events as count_events
reload(count_events)


# In[82]:

dg=d.groupby(grouping_keys)
dgda = np.asarray([dg.get_group(g).index[[0,-1]].values for g in dg.groups.keys()])


# In[ ]:

import concurrent.futures
N = 40000000
n = 64
with concurrent.futures.ThreadPoolExecutor(max_workers=32) as executor:
    # Start the load operations and mark each future with its
    future_to_url = {executor.submit(count_events.busy_sleep_nogil, N): i for i in range(n)}
    for future in concurrent.futures.as_completed(future_to_url):
        url = future_to_url[future]
        try:
            data = future.result()
        except Exception as exc:
            print('%d generated an exception: %s' % (url, exc))
        else:
            print('%d page is %d bytes' % (url, data))


# In[106]:

count_events.busy_sleep_nogil()


# In[ ]:




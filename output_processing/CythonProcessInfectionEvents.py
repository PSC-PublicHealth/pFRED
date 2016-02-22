
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


# The following cells reproduce the important bits of the cython cell magic.
# For guidance look here:
# - https://raw.githubusercontent.com/studer/ipython/master/IPython/extensions/cythonmagic.py

# In[4]:

from distutils.core import Distribution, Extension
from distutils.command.build_ext import build_ext
from Cython.Build import cythonize
import os

os.environ["CC"] = "gcc-5"
os.environ["CXX"] = "g++-5"
#os.environ["OMP_NUM_THREADS"] = "10"
os.environ["CFLAGS"] = '-I%s' % np.get_include()

dist = Distribution()
build_extension = build_ext(dist)
cyprinev_extension = Extension(
    name = 'count_events',
    sources = ['cyprinev/count_events.pyx'],
    include_dirs = [np.get_include()],
    language = 'c',
    #library_dirs = ['/usr/local/lib/gcc/5','/usr/local/lib'],
    #extra_compile_args=['-fopenmp'],
    #extra_link_args=['-fopenmp']
    #libraries = args.lib,
    #language = 'c++' if args.cplus else 'c',
)
build_extension.extensions = cythonize([cyprinev_extension], force=True)
build_extension.build_temp = 'cyprinev/build_temp'
build_extension.build_lib  = 'cyprinev'
build_extension.finalize_options()
build_extension.run()
import cyprinev.count_events as count_events
reload(count_events)


#  Note: this might be easier - for some reason the extension used above isn't
#  propagating/setting environment variables like "include_dirs", etc.
# 
# ```python
# pyximport.install(setup_args={"script_args":["--compiler=unix"],
#                               "include_dirs":numpy.get_include()},
#                   reload_support=True)
# ```

# To demonstrate threading without gil using concurrent.futures
# ```python
# count_events.busy_sleep_nogil()
# ```

# In[ ]:




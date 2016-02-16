
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
import numpy as np

extensions = [
        Extension(
            name = 'count_events',
            sources = ['count_events.pyx'],
            include_dirs = [np.get_include()],
            language = 'c',
            #library_dirs = ['/usr/local/lib/gcc/5','/usr/local/lib'],
            #extra_compile_args=['-fopenmp'],
            #extra_link_args=['-fopenmp']
            #libraries = args.lib,
            #language = 'c++' if args.cplus else 'c',
        ),
    ]

setup(
        name = 'Cythonized Processing of Infection Events',
        ext_modules = cythonize(extensions, force=True)
        )

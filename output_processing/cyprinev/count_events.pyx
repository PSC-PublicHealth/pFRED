#!python
#cython: boundscheck=False, wraparound=False

import cython
cimport cython
import numpy as np
cimport numpy as np

import concurrent.futures

###############################################################################

cpdef np.uint32_t[:,:] get_counts_from_group(np.uint32_t[:,:] rows, int ndays,
        event_map, state_map):

    cdef int exposed = event_map['exposed']
    cdef int infectious = event_map['infectious']
    cdef int symptomatic = event_map['symptomatic']
    cdef int recovered = event_map['recovered']
    cdef int susceptible = event_map['susceptible']

    cdef int N_i = state_map['N_i']
    cdef int S_i = state_map['S_i']
    cdef int E_i = state_map['E_i']
    cdef int I_i = state_map['I_i']
    cdef int Y_i = state_map['Y_i']
    cdef int R_i = state_map['R_i']
    cdef int IS_i = state_map['IS_i']

    cdef int N_p = state_map['N_p']
    cdef int S_p = state_map['S_p']
    cdef int E_p = state_map['E_p']
    cdef int I_p = state_map['I_p']
    cdef int Y_p = state_map['Y_p']
    cdef int R_p = state_map['R_p']
    cdef int IS_p = state_map['IS_p']

    cdef np.uint32_t[:,:] a = np.zeros([ndays, len(state_map)], dtype=np.uint32)
    cdef np.uint32_t[:] r

    cdef int i,d
    cdef int start_row = 0
    cdef int end_row = rows.shape[0]
    cdef np.uint32_t last_day = np.uint32(ndays - 1)
    cdef np.uint32_t num_days = np.uint32(ndays)
    cdef np.uint32_t NA = np.uint32(-1)

    with nogil:

        for i in xrange(start_row, end_row):
            
            r = rows[i,:]
            
            for d in xrange(0, ndays):
                a[d, N_p] += 1

            if r[exposed] == NA:
                for d in xrange(0, ndays): 
                    a[d, S_p] += 1

            else:
                a[r[exposed], E_i] += 1

                for d in xrange(r[exposed], r[infectious]):
                    a[d, E_p] += 1

                for d in xrange(0, r[exposed]):
                    a[d, S_p] += 1

                if r[susceptible] != NA:
                    if r[susceptible] == last_day:
                        a[r[susceptible], S_p] += 1
                        a[r[susceptible], S_i] += 1
                    elif r[susceptible] < last_day:
                        for d in xrange(r[susceptible], num_days):
                            a[d, S_p] += 1        

                if r[infectious] != NA:
                    a[r[infectious], I_i] += 1
                    for d in xrange(r[infectious], r[recovered]):
                        a[d, I_p] += 1
                    
                if r[symptomatic] != NA:
                    # NOTE: by default all symptomatics are infectious; this is
                    # a shortcut! 
                    a[r[symptomatic], Y_i] += 1
                    a[r[symptomatic], IS_i] += 1
                    for d in xrange(r[symptomatic], r[recovered]):
                        a[d, Y_p] += 1
                        a[d, IS_p] += 1

                if r[recovered] != NA:
                    a[r[recovered], R_i] += 1
                    if r[recovered] == last_day:
                        a[r[recovered], R_p] += 1
                    else:
                        for d in xrange(r[recovered], num_days):
                            a[d, R_p] += 1
    return a

###############################################################################

cdef inline int _busy_sleep(int n) nogil:
    cdef double tmp = 0.0

    while n > 0:
        # Do some CPU intensive useless computation to waste some time
        tmp = (n ** 0.5) ** 0.5
        n -= 1

def busy_sleep(int n):
    with nogil:
        _busy_sleep(n)
    return n

def busy_sleep_nogil():
    N = 40000000
    n = 64
    with concurrent.futures.ThreadPoolExecutor(max_workers=32) as executor:
        # Start the load operations and mark each future with its
        future_to_url = {executor.submit(busy_sleep, N): i for i in range(n)}
        for future in concurrent.futures.as_completed(future_to_url):
            url = future_to_url[future]
            try:
                data = future.result()
            except Exception as exc:
                print('%d generated an exception: %s' % (url, exc))
            else:
                pass
                #print('%d page is %d bytes' % (url, data))

###############################################################################












































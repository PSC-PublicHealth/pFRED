from __main__ import NA, DTYPE
from __main__ import state_map, event_map, group_map
from __main__ import dim_days_size, group_dims_sorted_indexes
from __main__ import a

last_day = DTYPE(dim_days_size - 1)
num_days = DTYPE(dim_days_size)

exposed = event_map['exposed']
infectious = event_map['infectious']
symptomatic = event_map['symptomatic']
recovered = event_map['recovered']
susceptible = event_map['susceptible']

N_i = state_map['N_i']
S_i = state_map['S_i']
E_i = state_map['E_i']
I_i = state_map['I_i']
Y_i = state_map['Y_i']
R_i = state_map['R_i']
IS_i = state_map['IS_i']

N_p = state_map['N_p']
S_p = state_map['S_p']
E_p = state_map['E_p']
I_p = state_map['I_p']
Y_p = state_map['Y_p']
R_p = state_map['R_p']
IS_p = state_map['IS_p']

def get_counts(r):
    group_indexes = []
    for g in group_dims_sorted_indexes:
        if r[g] == NA:
            return False
        else:
            group_indexes.append([r[g]])

    # always increment N_p (if groupings are not null)
    a[group_indexes + [[N_p]]] += 1

    if r[exposed] == NA:
        a[group_indexes + [[S_p]]] += 1
    else:
        a[group_indexes + [[E_i]] + [[r[exposed]]]] += 1
        a[group_indexes + [[E_p]] + [range(r[exposed], r[infectious])]] += 1
        
        a[group_indexes + [[S_p]] + [range(0, r[exposed])]] += 1
        if r[susceptible] != NA:
            if r[susceptible] < last_day:
                a[group_indexes + [[S_p]] + [range(r[susceptible], num_days)]] += 1
            elif r[susceptible] == last_day:
                a[group_indexes + [[S_p]] + [r[susceptible]]] += 1
                    
        if r[infectious] != NA:
            a[group_indexes + [[I_i]] + [[r[infectious]]]] += 1
            a[group_indexes + [[I_p]] + [range(r[infectious], r[recovered])]] += 1
            
        if r[symptomatic] != NA:
            a[group_indexes + [[Y_i]] + [[r[symptomatic]]]] += 1
            a[group_indexes + [[Y_p]] + [range(r[symptomatic], r[recovered])]] += 1
            # NOTE: by default all symptomatics are infectious; this is a shortcut 
            a[group_indexes + [[IS_i]] + [[r[symptomatic]]]] += 1
            a[group_indexes + [[IS_i]] + [range(r[symptomatic], r[recovered])]] += 1
           
        if r[recovered] != NA:
            a[group_indexes + [[R_i]] + [[r[recovered]]]] += 1
            if r[recovered] == last_day:
                a[group_indexes + [[R_p]] + [[r[recovered]]]] += 1
            else:
                assert(r[recovered] < last_day)
                a[group_indexes + [[R_p]] + [range(r[recovered], num_days)]] += 1
                
    return True

#!/usr/bin/env python

"""
FredEx: FRED Experiment Runner

Reads a yaml experiment specification, writes out parameters files
and submits jobs via qsub
"""

__version__ = '0.0.0'

import argparse, os, yaml, itertools, subprocess
from collections import OrderedDict
from numpy import arange

def run(config):
    expanded = OrderedDict()
    for k in config['experiment']:
        expanded[k] = []
        for i in range(0, len(config['experiment'][k])):
            if isinstance(config['experiment'][k][i], dict):
                expanded[k].extend(
                        arange(
                            config['experiment'][k][i]['from'],
                            config['experiment'][k][i]['to'],
                            config['experiment'][k][i]['by']))
            else:
                expanded[k].append(config['experiment'][k][i])

    x = expanded.keys()
    for t in itertools.product(*[expanded[k] for k in expanded]):
        yield ('.'.join(['%s.%s' % (str(x[i]), str(t[i])) for i in range(len(t))]),
               '\n'.join(['%s = %s' % (str(x[i]), str(t[i])) for i in range(len(t))]))


def write_params(base_string, t, outdir):
    paramfile = 'params.%s' % t[0]
    paramfile = os.path.join(outdir, paramfile)
    fred_out = os.path.join(outdir, 'OUT.%s' % t[0])
    reportfile = os.path.join(outdir, 'report.%s' % t[0])
    with open(paramfile, 'w') as f:
        f.write(base_string)
        f.write('\n\n########## EXPERIMENT PARAMS ##########\n\n')
        f.write('OUT = %s\n' % fred_out)
        f.write('event_report_file = %s\n' % reportfile)
        f.write(t[1])
        f.write('\n')
    return paramfile

def qsub(paramsfile, qsubfile, jobname):
    subprocess.call('qsub -v PARAMSFILE=%s -N %s %s' % (
        paramsfile, jobname, qsubfile), shell=True)

def main():
    parser = argparse.ArgumentParser(description='%s\nversion %s' % (
        __doc__, __version__),
        formatter_class=argparse.RawDescriptionHelpFormatter)
    
    parser.add_argument('-p', '--paramsfile', required=True,
            help='The default paramters file to base changes on')
    
    parser.add_argument('-c', '--config', required=True,
            help='Experiment config file')
 
    parser.add_argument('-q', '--qsubfile', required=True,
            help='qsub script')
 
    parser.add_argument('-o', '--outdir', required=True,
            help='Base output dir')
 
    args = parser.parse_args()

    outdir = os.path.abspath(args.outdir)

    try:
        os.mkdir(outdir)
    except:
        pass

    with open(args.paramsfile, 'r') as f:
        base_string = f.read()

    with open(args.config, 'r') as f:
        yaml_config = yaml.load(f)
    
    for t in run(yaml_config):
        jobname = t[0]
        paramsfile = write_params(base_string, t, outdir)




if __name__ == '__main__':
    main()

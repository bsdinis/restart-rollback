#!/usr/bin/python3.7

import csv
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from textwrap import wrap
import statistics as stat
from collections import defaultdict
import sys

def parse_file(fp):
    def add_to_dict(d, x, y):
        d['x'].append(x)
        d['y'].append(y)

    read, write, cond, rem  = defaultdict(lambda : []), defaultdict(lambda : []), defaultdict(lambda : []), defaultdict(lambda : [])
    for row in fp.readlines():
        op, lat, sz = row.strip().split(', ')
        if   op == 'READ': add_to_dict(read, int(sz), float(lat))
        elif op == 'WRITE': add_to_dict(write, int(sz), float(lat))
        elif op == 'COND': add_to_dict(cond, int(sz), float(lat))
        elif op == 'REM': add_to_dict(rem, int(sz), float(lat))

    def filter_nonempty(labls, data, labl, l):
        if len(l) > 0:
            labls.append(labl)
            data.append(l)

    labls = list()
    data = list()
    filter_nonempty(labls, data, 'Read', read)
    filter_nonempty(labls, data, 'Write', write)
    filter_nonempty(labls, data, 'Cond', cond)
    filter_nonempty(labls, data, 'Rem', rem)
    return  labls, data

def plot(x, y, title=None, outfile=None):
    fig = plt.figure(figsize=(15, 9))
    # Create an axes instance
    ax = fig.add_subplot(111)
    # Create the boxplot
    bp = ax.scatter(x, y)
    ax.set_ylabel('Latency ($\mu s$)')
    ax.set_xlabel('Blob Size (KB)')

    if title:
        plt.title(title)
    # Save the figure
    if outfile:
        fig.savefig(outfile, bbox_inches='tight')

def plot_all(datas, title=None, outfile=None, labels=None):
    fig = plt.figure(figsize=(15, 9))
    # Create an axes instance
    ax = fig.add_subplot(111)
    ax.set_ylabel('Latency ($\mu s$)')
    ax.set_xlabel('Blob Size (KB)')

    # Create the boxplots
    if labels:
        for labl, data in zip(labels, datas):
            ax.scatter(data['x'], data['y'], label=labl)
        plt.legend(loc='upper left')
    else:
        for data in datas:
            ax.scatter(data['x'], data['y'])

    if title:
        plt.title(title)

    # Save the figure
    if outfile:
        fig.savefig(outfile, bbox_inches='tight')

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: ./plot.py [csv file]")
        sys.exit(-1)

    title = sys.argv[1].strip().split('.')[0]
    with open(sys.argv[1], 'r') as f:
        labls, datas = parse_file(f)
        for labl, data in zip(labls, datas):
            plot(data['x'], data['y'],\
                    '{}: {} Latency'.format(title, labl),\
                    '{}_{}.svg'.format(title, labl.lower()))
        plot_all(datas, title+': General Overview', title+'_general.svg', labls)



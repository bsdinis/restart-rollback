#!/usr/bin/python3.7

import csv
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
from textwrap import wrap

import statistics

import sys

def parse(fp):
    read, write, cond, rem  = list(), list(), list(), list()
    for row in fp.readlines():
        op, lat, *_ = row.strip().split(', ')
        if   op == 'READ': read.append(float(lat))
        elif op == 'WRITE': write.append(float(lat))
        elif op == 'COND': cond.append(float(lat))
        elif op == 'REM': rem.append(float(lat))

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

    return labls, data

def stats(labls, data_to_plot):
    def stat_per_label(labl, data):
        data = sorted(data)
        print("{}:\t| {}".format(labl, len(data)))
        print("Mean:\t| {} us".format(statistics.mean(data)))
        print("Stdev:\t| {} us".format(statistics.stdev(data)))
        print("25%:\t| {} us".format(data[len(data) // 4]))
        print("Median:\t| {} us".format(statistics.median(data)))
        print("75%:\t| {} us".format(data[3 * len(data) // 4]))
        print("Range:\t| {} - {} us".format(data[0], data[-1]))
        print("===============================")

    for labl, data in zip(labls, data_to_plot):
        stat_per_label(labl, data)


def plot(labls, data_to_plot, title=None, outfile=None):
    fig = plt.figure(1, figsize=(9, 6))
    #fig = plt.figure(1, figsize=(15, 9))

    # Create an axes instance
    ax = fig.add_subplot(111)

    # Create the boxplot
    ax.boxplot(data_to_plot,
            labels=['\n'.join(wrap(lbl,9)) for lbl in labls],
            showfliers=False
            )

    ax.set_ylabel('Latency ($\mu s$)')

    # Save the figure
    if title:
        plt.title(title)
    if outfile:
        fig.savefig(outfile, bbox_inches='tight')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("usage: ./plot.py [csv file]")
        sys.exit(-1)

    out= ''.join([c for idx, c in enumerate(sys.argv[1]) if '.' not in sys.argv[1][:idx+1]])
    with open(sys.argv[1], 'r') as f:
        labls, data = parse(f)
        stats(labls, data)
        plot(labls, data, title=out.replace('_', ' '), outfile=out+'.svg')



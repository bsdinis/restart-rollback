#!/usr/bin/python3

import csv
import numpy as np
from textwrap import wrap


import statistics

import sys

def parse(fp):
    data = dict();
    for row in fp.readlines():
        if row.strip()[0] == '#':
            continue;
        op, lat, *_ = row.strip().split(', ')
        op = op.lower()
        if op in data:
            data[op].append(float(lat))
        else:
            data[op] = [float(lat)]

    for op in data:
        data[op] = sorted(data[op])

    return data

def stats(data, show=True):
    def stat_per_label(labl, data, show):
        stats = {
                'size': len(data),
                'mean': statistics.mean(data),
                'stddev': statistics.stdev(data),
                '1st_percentile': data[len(data) // 100],
                'median': statistics.median(data),
                '99th_percentile': data[(3 * len(data)) // 4],
                'range': (data[0], data[-1],),
                'unit': '\mu s',
                }

        if show:
            print("{}:\t| {}".format(labl, stats['size']))
            print("Mean:\t| {} us".format(stats['mean']))
            print("Stdev:\t| {} us".format(stats['stddev']))
            print("1%:\t| {} us".format(stats['1st_percentile']))
            print("Median:\t| {} us".format(stats['median']))
            print("99%:\t| {} us".format(stats['99th_percentile']))
            print("Range:\t| {} - {} us".format(stats['range'][0], stats['range'][1]))
            print("===============================")

        return stats


    stats = dict()
    for labl, d in data.items():
        stats[labl] = stat_per_label(labl, d, show)

    return stats

def resizable(stats):
    return all(stats[labl]['median'] for labl in stats);

def resize_to(stats, new_unit, factor):
    for labl in stats:
        stats[labl]['unit'] = new_unit
        for stat in stats[labl]:
            if stat == 'size' or stat == 'unit': continue;
            if stat == 'range':
                stats[labl][stat] = tuple(map(lambda x: x * factor, stats[labl][stat]));
            else:
                stats[labl][stat] *= factor;

    return stats

def export_dat(statmap, outfile):
    with open(outfile, "w") as outf:
        stats = dict() # op -> list of vals
        labels = statmap.keys()
        for i, lbl in enumerate(labels):
            for op in sorted(statmap[lbl].keys()):
                if op in stats:
                    #stats[op].append(statmap[lbl][op]['median'])
                    stats[op].append((statmap[lbl][op]['median'], statmap[lbl][op]['1st_percentile'], statmap[lbl][op]['99th_percentile']))
                else:
                    #stats[op] = [0] * i + [statmap[lbl][op]['median']]
                    stats[op] = [(0, 0, 0)] * i + [(statmap[lbl][op]['median'], statmap[lbl][op]['1st_percentile'], statmap[lbl][op]['99th_percentile'])]

            for op in stats:
                if op not in sorted(statmap[lbl].keys()):
                    #stats[op].append(0)
                    stats[op].append((0, 0, 0))

        print("Operation\t" + '\t'.join('"' + a + '"' + "\tmin\tmax\t" for a in labels), file=outf)
        for op in stats:
            print('\"{}\"'.format(op) + '\t' +\
                    '\t'.join("{}\t{}\t{}".format(a[0], a[1], a[2])\
                    for a in stats[op]), file=outf)
            # todo: tuple case?


def export_gnu(count, title, unit):
    template =\
'''
set terminal postscript eps colour size 12cm,8cm enhanced font 'Helvetica,20'
set output '{}'

set key outside above

set border linewidth 0.75

set style data histogram
set style histogram cluster gap 1 errorbars lw 2

set style fill pattern 4 border rgb "black"
set auto x
set yrange [0:*]
set ylabel \'Latency ({})\'
'''

    def gen_plot(count, title):
        res = 'plot for [i=2:{}:3] \'{}\' using i:i+1:i+2:xtic(1) title col(i)'.format(2 + 3 * (count - 1), title + '.dat')
        return res

    with open(title + '.gnu', 'w') as outf:
        print(template.format(title + '.eps', unit), file = outf)
        print(gen_plot(count, title), file = outf)




if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("usage: ./plot.py title [csv file1] ... [csv file n]")
        sys.exit(-1)

    total_stats = dict();
    for filename in sys.argv[2:]:
        with open(filename, 'r') as f:
            data = parse(f)
            stat = stats(data)
            title = ' '.join(filename.strip().split('.')[:-1]).replace('_', ' ') # remove extension and replace dots with spaces
            total_stats[title] = stat

    unit = '\mu s'
    if all(resizable(total_stats[title]) for title in total_stats):
        unit = 'ms'
        for title in total_stats:
            total_stats[title] = resize_to(total_stats[title], unit, 1. / 1000.)

    export_dat(total_stats, sys.argv[1]+'.dat')
    export_gnu(len(sys.argv[2:]), sys.argv[1], unit)

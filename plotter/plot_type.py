#!/usr/bin/env python3

import time
import os
import sys
import subprocess
import math
import re
import ast

from template import *
from functools import reduce

def localtime(): return time.asctime(time.localtime(time.time()))
def create_dir(path):
    """
    creates a dir if it doesn't exist
    """
    if not os.path.isdir(path): os.mkdir(path)

def normalize_dir(string): return string.replace(' ', '_') if string else None

def resizable_y(ys):
    return all(all(y > 2000 for y in yprime) for yprime in ys)

def resize_y(ys, factor):
    return [[y * factor for y in yprime] for yprime in ys]

def resizable_x(xs):
    return all(x > 2000 for x in xs)

def resize_x(xs, factor):
    return [x * factor for x in xs]

class Plot:
    def __init__(self, dirname, name, xs, ys, xlabel = None, ylabel = None, title = None):
        """
        Assumptions:
          xs is an array of n points
          ys is an array of n tuples
          (a matrix is a list of lists, or equivalent)
        """
        self.dirname = dirname;
        self.name = name;
        self.xs = xs; # one dimensional
        self.ys = ys; # multi dimensional
        self.xlabel = xlabel;
        self.ylabel = ylabel;
        self.title = normalize_dir(title)
        create_dir(dirname)


    def get_full_file(self, ext): return self.dirname + '/' + ext + '/' + self.name + '.' + ext
    def get_file(self, ext): return self.name + '.' + ext
    def get_dir(self, ext): return self.dirname + '/' + ext

    def __print_comment_header(self, f, filename, what = None, comment_char = '#'):
        print("{} {}"\
                .format(comment_char, filename), file=f)
        if self.title and what: print("{} {} {}"\
                .format(comment_char, self.title, what), file=f)
        print("{} Generated by plotter.py at: {}"\
                .format(comment_char, localtime()), file=f)
        if self.xlabel: print("{} x axis: {}"\
                .format(comment_char, self.xlabel), file=f)
        if self.ylabel: print("{} y axis: {}"\
                .format(comment_char, self.xlabel), file=f)

    def save_to_dat(self):
        create_dir(self.get_dir('dat'))
        with open(self.get_full_file('dat'), 'w') as f:
            self.__print_comment_header(f, self.get_file('dat'), 'data file')
            print("\n\n# begin", file = f)
            strs = [str(x) + ' ' + ' '.join(str(y) for y in ys)\
                    for x, ys in zip(self.xs, self.ys)]
            print('\n'.join(sorted(strs, key = lambda x: ast.literal_eval(x.split()[0]))), file = f)
            print("\n\n# end", file = f)
            f.close()

    def save_to_gnuplot(self):
        self.save_to_dat()
        create_dir(self.get_dir('gnu'))
        with open(self.get_full_file('gnu'), 'w') as f:
            self.__print_comment_header(f, self.get_file('gnu'), 'gnuplot file')
            print("\n\n# begin", file = f)
            single_plot(self, f)
            print("\n\n# end", file = f)
            f.close()

    def save_to_png(self):
        self.save_to_gnuplot()
        create_dir(self.get_dir('png'))
        launch_gnuplot(self.get_full_file('gnu'))

def multiplot_save_to_gnuplot(name, plotlist):
    if len(plotlist) == 0: return
    dirname = plotlist[0].dirname
    create_dir(dirname)
    for plot in plotlist:
        plot.save_to_dat()

    create_dir(dirname + '/gnu')
    with open(dirname + '/gnu/' + name + '.gnu', 'w') as f:
        multiple_plot(plotlist, name, f)
        f.close()

def multiplot_save_to_png(name, plotlist):
    if len(plotlist) == 0: return
    multiplot_save_to_gnuplot(name, plotlist)
    dirname = plotlist[0].dirname
    create_dir(dirname + '/png')
    launch_gnuplot(dirname + '/gnu/' + name + '.gnu')

def launch_gnuplot(gnuplot_script):
    try:
        subprocess.run(['gnuplot', gnuplot_script], check=True)
    except subprocess.CalledProcessError:
        print("Failed to call gnuplot with {}".format(gnuplot_script),\
                file = sys.stderr)
        exit()

def multiple_plot(plotlist, name, f):
    """
    This generates the gnuplot file for a list of Plot's
    The lines are overlapped
    Assumes the labels of the first plot
    Finds the adequate ranges
    """
    print_header(f, plotlist[0].dirname + '/png/' + name + '.png')
    for idx, plot in enumerate(plotlist):
        color = color_list[chosen_colors[idx % len(chosen_colors)]]
        print_linetype(f, idx + 1, color)

    xranges = map(lambda plot: (min(plot.xs), max(plot.xs)), plotlist)
    yranges = map(lambda plot: (min(min(plot.ys, key=lambda x: min(x))), max(max(plot.ys, key=lambda x: max(x)))), plotlist)

    xran = reduce(lambda x, y: (min(x[0], y[0]), max(x[1], y[1])),\
            xranges, (math.inf, -math.inf))
    yran = reduce(lambda x, y: (min(x[0], y[0]), max(x[1], y[1])),\
            yranges, (math.inf, -math.inf))

    print_axis(f,
            xlabel = plotlist[0].xlabel,
            ylabel = plotlist[0].ylabel,
            xrangex = xran, yrangex = yran)

    print_plot(f,\
            (\
            (plot.title, plot.get_full_file('dat'),\
            idx + 1, len(plot.ys[0]) > 1)\
            for idx, plot in enumerate(sorted(plotlist, key = lambda x: x.title))\
            ))

def single_plot(plot, f): multiple_plot((plot, ), plot.name, f)

if __name__ == '__main__':
    plot1 = Plot('test_plots', 'plot1',\
            [1, 2, 3, 4, 5],\
            [(2,), (1,), (4,), (5,), (3,)],\
            xlabel = 'x points', ylabel = 'y points',\
            title = 'Sample plot 1')
    plot1.save_to_dat()
    plot1.save_to_gnuplot()
    plot1.save_to_png()
    print("Plot 1: Success")

    plot2 = Plot('test_plots', 'plot2',\
            [1, 2, 3, 4, 5],\
            [(1, 0, 2,), (2, 1, 3,), (3, 1, 5), (4, 2, 5), (5, 3, 6)],\
            xlabel = 'x points', ylabel = 'y points',\
            title = 'Sample plot 2')
    plot2.save_to_dat()
    plot2.save_to_gnuplot()
    plot2.save_to_png()
    print("Plot 2: Success")


    plot3 = Plot('test_plots', 'plot3',\
            [1, 2, 3, 4, 5],\
            [(5,), (3,), (2,), (1,), (4,)],\
            xlabel = 'x points', ylabel = 'y points',\
            title = 'Sample plot 3')
    plot3.save_to_png()
    print("Plot 3: Success")

    multiplot_save_to_png('multiplot', [plot1, plot2, plot3])
    print("Multiplot: Success")
    print("Success")

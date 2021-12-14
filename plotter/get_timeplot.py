#!/usr/bin/env python3

import sys
from parser import *

if __name__ == '__main__':
    assert len(sys.argv) != 1, f'usage: {sys.argv[0]} dump_file'
    ticks = separate_by_op(get_ticks(sys.argv[1]))

    for op, ticklist in ticks.items():
        print("# {}".format(op))
        for tick in ticklist:
            print("# {}".format(tick.load()))
            first_tick = None
            for point in tick:
                if first_tick == None: first_tick = point.id()
                print('{:<10} {:<10}'\
                        .format(point.id() - first_tick, point.throughput()))

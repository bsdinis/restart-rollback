#!/usr/bin/env python3

import sys
from parser import *

if __name__ == '__main__':
    assert len(sys.argv) != 1, f'usage: {sys.argv[0]} dump_file'
    ticks = separate_by_op(get_ticks(sys.argv[1]))
    for op, ticklist in ticks.items():
        print("# {}".format(op))
        for tick in ticklist:
            (first, median, third) = tick.throughput_quantiles()
            print('{:<10} {:<10} {:<10} {:<10}'\
                    .format(tick.load(), median, first, third))

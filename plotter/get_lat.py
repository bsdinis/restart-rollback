#!/usr/bin/env python3

import sys
from parser import *

if __name__ == '__main__':
    assert len(sys.argv) != 1, f'usage: {sys.argv[0]} dump_file'
    operations = separate_by_op(get_operations(sys.argv[1]))
    for op, oplist in operations.items():
        print("# {}".format(op))
        (first, median, third) = oplist.latency_quantiles()
        print('{:<10} {:<10} {:<10} {:<10}'\
                .format(oplist.load(), median, first, third))

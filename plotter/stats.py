#!/usr/bin/python3
#!/bin/python3
import sys
import re
import operator
import parser
from functools import reduce

def print_statistics(operations):
    def print_line():
        print("+{}+{}+{}+{}+".format('-'*13, '-' * 8, '-'* 11, '-'* 10))

    def print_row(op, total, succper, failper):
        print("| {:^11} | {:>6} | {:>8.4f}% | {:>7.4f}% |"\
                .format(op, total, succper, failper))
        print_line()

    def op_statistics(ops):
        total = reduce(operator.add, map(lambda x: x.total(), ops), 0)
        successes = reduce(operator.add, map(lambda x: x.successes(), ops), 0)
        failures = reduce(operator.add, map(lambda x: x.failures(), ops), 0)
        if total == 0: return

        succper = round(successes / total, 6) * 100
        failper = round(failures / total, 6) * 100
        op_name = repr(ops[0].op())
        print_row(op_name, total, succper, failper)


    flat_ops = [el for subl in operations for el in subl]
    total = reduce(operator.add, map(lambda x: x.total(), flat_ops), 0)
    if total == 0:
        print("Empty dump")
        exit()

    successes = reduce(operator.add, map(lambda x: x.successes(), flat_ops), 0)
    failures = reduce(operator.add, map(lambda x: x.failures(), flat_ops), 0)
    succper = round(successes/ total, 6) * 100
    failper = round(failures / total, 6) * 100
    print("{:>7.4f}".format(succper))
    print("{:>7.4f}".format(failper))

    print_line()
    print("| {:^11} | {:^6} | {:^9} | {:^8} |"\
            .format('Operation', 'Total', 'Successes', 'Failures'))
    print_line()
    print_row('All', total, succper, failper)

    for ops in operations:
        op_statistics(ops)

if __name__ == '__main__':
    assert len(sys.argv) != 1, 'usage: {} dump_file'.format(sys.argv[0])
    operations = parser.separate_by_op(parser.get_operations(sys.argv[1]))
    print_statistics(operations.values())

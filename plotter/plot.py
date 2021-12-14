#!/usr/bin/env python3

import sys
import plotlib

if __name__ == '__main__':
    assert len(sys.argv) != 1, f'usage: {sys.argv[0]} dump_file'

    config = {
            'test_name': sys.argv[1].split('.')[0],
            'filename': sys.argv[1],
            'latency': True,
            'throughput': True,
            'timeplots': False,
            'read_latency': False,
            'write_latency': False,
            'conditional_latency': False,
    }

    plotlib.run(config)

#!/usr/bin/env python3

import csv
import sys
from collections import defaultdict
from functools import reduce

from util_types import *

def parse_event(args):
    lineno = args[0] + 1
    line = args[1]
    assert len(line) == 5, "Bad format on line {}: wrong number of arguments: {} (expected 5):\n{}".format(\
            lineno, len(line), line)

    ts = int(line[0])
    load = int(line[1])
    tick_duration = int(line[2])
    ticket = int(line[3])

    if line[4].lower() == "read":
        op = OperationType.RD
    elif line[4].lower() == "write":
        op = OperationType.WR
    elif line[4].lower() == "cond_write":
        op = OperationType.CWR
    else:
        raise ValueError("Unknown operation on line {}: {}\n{}"\
                .format(lineno, line[3], line))

    return Invocation(ts, load, tick_duration, ticket, op, lineno) if line[4].islower() else\
            Reply(ts, load, tick_duration, ticket, op, lineno)

def get_events(filename):
    """
    Parses a file and returns the list of events
    An event may be an invocation or a reply
    """
    with open (filename, 'r') as f:
        reader = csv.reader(filter(lambda row: row.strip()[0] != '#', f), strict = True, skipinitialspace=True)
        l = sorted(map(parse_event, enumerate(reader)))
        events = dict()
        for el in l:
            if el.load not in events:
                events[el.load] = [ el ]
            else:
                events[el.load].append(el)

        return events

def _parse_or_cleanly(arg):
    if isinstance(arg, dict):
        return arg
    elif isinstance(arg, str):
        return get_events(arg)
    else:
        raise ValueError("Invalid argument type for function")


def get_ticks(arg):
    """
    Returns list of TickLists

    Consumes either:
      - a list of events
      - a filename from which is possible to parse a list of events using `get_events`
    """

    event_map = _parse_or_cleanly(arg)
    tick_lists = list()
    for load in event_map:
        events = event_map[load]
        ticks = defaultdict(lambda: []) # key -> operation
        old_idx = 0
        tick_id = events[0].get_tick()
        tick_width = events[0].width
        tick_op = events[0].op
        for idx, event in enumerate(events):
            if event.op != tick_op or event.width != tick_width or event.get_tick() != tick_id:
                if idx != old_idx: ticks[tick_op].append(Tick(events[old_idx:idx]))
                tick_id = event.get_tick()
                tick_width = event.width
                tick_op = event.op
                old_idx = idx

        for op, tick in ticks.items():
            tick_lists.append(TickList(tick))

    return tick_lists

def get_operations(arg):
    """
    Returns list of OperationLists

    Consumes either:
      - a list of events
      - a filename from which is possible to parse a list of events using `get_events`

    Panics if it finds a mismatched reply, which should never happen,
    while invocations can fail to return
    """

    event_map = _parse_or_cleanly(arg)
    operation_lists = list()

    for load in event_map:
        operations = defaultdict(lambda: [])
        mismatched = defaultdict(lambda: dict())
        events = event_map[load]
        for event in events:
            op = event.op
            ticket = event.ticket
            if isinstance(event, Invocation):
                mismatched[op][ticket] = event
            elif isinstance(event, Reply):
                # this works because of the overloaded __eq__ operator
                if ticket not in mismatched[op]:
                    raise ValueError("Reply without an invocation: {}".format(event))
                invoc = mismatched[op].pop(ticket)
                operations[invoc.op].append(Operation(invoc, event))
            else:
                raise TypeError("An event should only be a reply or an invocation")

        for op in operations:
            operation_lists.append(OperationList(operations[op], mismatched[op].values()))

    return operation_lists

def separate_by_op(lists):
    """
    Convert a list of OperationList's and convert to map
    keyed by the operation type
    """
    separated = defaultdict(lambda : [])
    for l in lists: separated[l.op()].append(l)
    return separated

def separate_by_load(lists):
    """
    Convert a list of OperationList's and convert to map
    keyed by the operation type
    """
    separated = defaultdict(lambda : [])
    for l in lists: separated[l.load()].append(l)
    return separated

def separate_ticks_by_load(lists):
    def merge(ticklists):
        return TickList(reduce(lambda x, y: x + y, map(lambda x: x.ticks, ticklists), []))
    separated = separate_by_load(lists)
    return { load: merge(ticklists) \
            for load, ticklists in separated.items() }



if __name__ == '__main__':
    assert len(sys.argv) != 1, 'usage: {} dump_file'.format(sys.argv[0])
    filename = sys.argv[1]
    events = get_events(filename)
    get_operations(events)
    get_ticks(events)
    print("File {} was successfully parsed".format(filename))

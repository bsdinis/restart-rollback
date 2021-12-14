from enum import Enum
from functools import total_ordering

import numpy as np
import math
import statistics

class OperationType(Enum):
    RD  = 1
    WR  = 2
    CWR = 3

    def __repr__(self):
        if self == OperationType.RD: return 'Read'
        elif self == OperationType.WR: return 'Write'
        elif self == OperationType.CWR: return 'Conditional'

    def __str__(self): return repr(self)

@total_ordering
class Event:
    def __init__(self, ts, offered_load, tick_duration, ticket, operation, lineno):
        assert isinstance(ts, int), "timestamp is not an integer on line {}".format(lineno)
        assert isinstance(offered_load, int), "load is not an integer on line {}".format(lineno)
        assert isinstance(ticket, int), "timestamp is not an integer on line {}".format(lineno)
        assert isinstance(operation, OperationType), "operation is not an OperationType on line {}".format(lineno)
        self.ts = ts
        self.ticket = ticket
        self.load = offered_load
        self.width = tick_duration
        self.op = operation
        self.lineno = lineno

    def __lt__(self, other):
        return self.load < other.load and\
                self.ticket < other.ticket and\
                self.ts < other.ts

    def __eq__(self, other):
        return self.load == other.load and\
                self.ticket == other.ticket

    def get_tick(self):
        """
        here, the width is the number of microseconds in a tick
        essentially, once we integer divide the ts by the width
        we get the required values
        """
        return self.ts // self.width



class Invocation(Event):
    def __repr__(self):
        return "{} invocation @ {}: {} [{} ops/sec] on line {}".format( self.op, self.ts, self.ticket, self.load, self.lineno)

class Reply(Event):
    def __repr__(self):
        return "{} reply @ {}: {} [{} ops/sec] on line {}".format( self.op, self.ts, self.ticket, self.load, self.lineno)


@total_ordering
class Operation:
    def __init__(self, invoc, reply):
        assert invoc.ticket == reply.ticket,\
                "Cannot create operation from {} and {}, the tickets don't match".format(\
                invoc, reply)

        assert invoc.ts < reply.ts,\
                "Cannot create operation from {} and {}, time travelling reply".format(\
                invoc, invoc.lineno, reply, reply.linenno)

        assert invoc.op == reply.op,\
                "Cannot create operation from {} and {}, mismatched operation".format(\
                invoc, invoc.lineno, reply, reply.linenno)

        assert invoc.load == reply.load,\
                "Cannot create operation from {} and {}, the loads don't match".format(\
                invoc, invoc.lineno, reply, reply.linenno)

        self.ticket = invoc.ticket
        self.load = invoc.load
        self.op = invoc.op
        self.begin = invoc.ts
        self.end = reply.ts
        self.lines = (invoc.lineno, reply.lineno)

    def latency(self): return self.end - self.begin

    def __lt__(self, other):
        return self.load < other.load and\
                self.ticket < other.ticket and\
                self.begin < other.begin

    def __eq__(self, other):
        return self.load == other.load and\
                self.ticket == other.ticket and\
                self.begin == other.begin

    def __repr__(self):
        return "{}: {} [{} us] [{} ops/sec] on lines {} - {}".format( self.op, self.ticket, self.latency(), self.load, self.lines[0], self.lines[1])



class OperationList:
    def __init__(self, operation_list, mismatched):
        assert len(operation_list),\
                "Cannot instantiante OperationList from an empty list"
        self.operations = operation_list
        self.mismatched = mismatched
        self.no_outliers = remove_outliers(self.operations, lambda x: x.latency())

    def op(self):   return self.operations[0].op
    def load(self): return self.operations[0].load

    def successes(self): return len(self.operations)
    def failures(self): return len(self.mismatched)
    def total(self): return self.successes() + self.failures()

    def latency_quantiles(self):
        quant = quantiles(map(lambda x: x.latency(), self.no_outliers), 4)
        return (quant[1], quant[0], quant[2],)


class Tick:
    def __init__(self, event_list):
        assert len(event_list) > 0,\
                "Cannot create a tick from an empty list"
        self.events = event_list

    def id(self):    return self.events[0].get_tick()
    def width(self): return self.events[0].width
    def op(self):    return self.events[0].op
    def load(self):  return self.events[0].load

    def throughput(self):
        count = len(tuple(\
                filter(lambda x: isinstance(x, Reply), self.events)))

        return count / (float(self.width()) / 1000000)

class TickList:
    def __init__(self, ticks):
        assert len(ticks),\
                "Cannot instantiante TickList from an empty list"

        self.ticks = ticks
        self.no_outliers = remove_outliers(self.ticks, lambda x: x.throughput())

    def op(self):    return self.ticks[0].op()
    def load(self):  return self.ticks[0].load()
    def width(self): return self.ticks[0].width()

    def throughput_quantiles(self, n=4):
        quant = quantiles(map(lambda x: x.throughput(), self.no_outliers), 4)
        return (quant[1], quant[0], quant[2],)

    def __iter__(self):
        return TickListIterator(self)

class TickListIterator:
    def __init__(self, ticklist):
        self._list = ticklist.ticks
        self._idx = 0

    def __next__(self):
        if self._idx >= len(self._list):
            raise StopIteration
        result = self._list[self._idx]
        self._idx += 1
        return result


def quantiles(arr, n=100):
    return (np.percentile(sorted(arr), tuple((100 * i)//n for i in range(1, n))))

def remove_outliers(arr, func, sigma_distance=4):
    if len(arr) == 0: return arr
    avg = statistics.mean(map(func, arr))
    stdev = statistics.stdev(map(func, arr))
    if stdev == float(0): return arr
    return filter(lambda x: abs(func(x) - avg) < sigma_distance * stdev, arr)


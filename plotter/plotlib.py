from parser import *
from plot_type import *
import sys

def lists_to_plot(test_name, lists, title_key, ykey,\
        xlabel = None, ylabel = None, title = None,
        alt_xlabel = None, alt_ylabel = None):
    xs, ys = list(), list()
    for l in lists:
        xs.append(l.load())
        ys.append(ykey(l))

    xx_label = xlabel
    if resizable_x(xs):
        if alt_xlabel:
            xs = resize_x(xs, 1. / 1000.)
            xx_label = alt_xlabel
        else:
            print("Wanted to resize, but no alternative label for {}".format(xlabel))

    yy_label = ylabel
    if resizable_y(ys):
        if alt_ylabel:
            ys = resize_y(ys, 1. / 1000.)
            yy_label = alt_ylabel
        else:
            print("Wanted to resize, but no alternative label for {}".format(ylabel))

    return Plot(test_name, title_key(l), xs, ys,\
                xlabel = xx_label,\
                ylabel = yy_label,\
                title  = title)

def oplists_to_latplot(test_name, oplists):
    return lists_to_plot(test_name, oplists,\
            lambda l: repr(l.op()).lower() + '_lat',\
            lambda l: l.latency_quantiles(),\
            xlabel = 'Offered Load (ops/s)',\
            ylabel = 'Latency (\mu s)',\
            alt_xlabel = 'Offered Load (Kops/s)',\
            alt_ylabel = 'Latency (ms)',\
            title  = repr(oplists[0].op()))

def ticklists_to_throughplot(test_name, ticklists):
    return lists_to_plot(test_name, ticklists,\
            lambda l: repr(l.op()).lower() + '_throughput',\
            lambda l: l.throughput_quantiles(),\
            xlabel = 'Offered Load (ops/s)',\
            ylabel = 'Throughput (ops/s)',\
            alt_xlabel = 'Offered Load (Kops/s)',\
            alt_ylabel = 'Throughput (Kops/s)',\
            title  = repr(ticklists[0].op()))

def gen_latency_plots(operations, test_name):
    test_name = normalize_dir(test_name)
    return { op: oplists_to_latplot(test_name, oplists)\
            for op, oplists in operations.items() }

def gen_operation_throughput_plots(ticklists, test_name):
    test_name = normalize_dir(test_name)
    return { op: ticklists_to_throughplot(test_name, tick)\
            for op, tick in ticklists.items() }

def gen_throughput_plot(ticks, test_name):
    test_name = normalize_dir(test_name)
    xs, ys = list(), list()
    for x, y in ticks.items():
        xs.append(x)
        ys.append(y.throughput_quantiles())

    xx_label = 'Offered Load (ops/s)'
    if resizable_x(xs):
        xs = resize_x(xs, 1. / 1000.)
        xx_label = 'Offered Load (Kops/s)'

    yy_label = 'Throughput (ops/s)'
    if resizable_y(ys):
        ys = resize_y(ys, 1. / 1000.)
        ylabel = 'Throughput (Kops/s)'

    return Plot(test_name, 'throughput', xs, ys,\
            xlabel = xx_label,\
            ylabel = yy_label,\
            )

def gen_timeplots(ticks, test_name):
    test_name = normalize_dir(test_name)
    plotlist = list()
    for tick in ticks:
        xs, ys = list(), list()
        first_tick = None
        for x in tick:
            if first_tick == None: first_tick = x.id()
            # time in milliseconds
            xs.append((x.id() - first_tick) * x.width() / 1000)
            ys.append((x.throughput(), ))

        plotlist.append(Plot(test_name,\
                '{}_{}_timeplot'.format(repr(tick.op()).lower(), tick.load()),\
                xs, ys,\
                xlabel = 'Time (ms)',\
                ylabel = 'Throughput (ops/s)',\
                ))
    return plotlist

def plot_joint_throughput(throughplots):
    multiplot_save_to_png('full_throughput', throughplots)

def plot_joint_lat(latplots):
    multiplot_save_to_png('latency', latplots)

def plot_separate(plotlist):
    for plot in plotlist:
        plot.save_to_png()

def run(config):
    test_name = config['test_name']
    events = get_events(config['filename'])
    if len(events) == 0:
        print("Empty dump")
        exit()
    operations = separate_by_op(get_operations(events))
    ticks = get_ticks(events)

    latplots = gen_latency_plots(operations, test_name)
    throughplots = gen_operation_throughput_plots(separate_by_op(ticks), test_name)
    throughput_plot = gen_throughput_plot(separate_ticks_by_load(ticks), test_name)
    timeplots = gen_timeplots(ticks, test_name)

    if config['latency']:
        plot_joint_lat(tuple(latplots.values()))

    if config['read_latency']:
        if OperationType.RD in latplots: latplots[OperationType.RD].save_to_png()
        if OperationType.RD in throughplots: throughplots[OperationType.RD].save_to_png()

    if config['write_latency']:
        if OperationType.WR in latplots: latplots[OperationType.WR].save_to_png()
        if OperationType.WR in throughplots: throughplots[OperationType.WR].save_to_png()

    if config['conditional_latency']:
        if OperationType.CWR in latplots: latplots[OperationType.CWR].save_to_png()
        if OperationType.CWR in throughplots: throughplots[OperationType.CWR].save_to_png()

    if config['throughput']:
        #throughput_plot.save_to_png()
        plot_joint_throughput(tuple(throughplots.values()))

    if config['timeplots']:
        plot_separate(timeplots)



if __name__ == '__main__':
    assert len(sys.argv) != 1, f'usage: {sys.argv[0]} dump_file'
    config = {
            'test_name': sys.argv[1].split('.')[0],
            'filename': sys.argv[1],
            'latency': True,
            'throughput': True,
            'timeplots': True,
            'read_latency': True,
            'write_latency': True,
            'conditional_latency': True,
    }


    run(config)

# basicQP: clnt

Client lib and tests.
This builds a client library for `basicQP`: `libbasicQP`.
Additionally it builds four applications using that library:

  * `qp_test`: tests the *functionality* of the QP
  * `close_remote`: closes a remote QP
  * `latency_test`: benchmarks the latency of an operation
  * `throughput_test`: drives a throughput benchmark on a specific operation

## Dependencies

Here is a (non-complete) list of dependencies to build this QP:

  * openssl-devel
  * [flatbuffers](https://github.com/google/flatbuffers): it is recommended to build and install from source
  * g++ and clang++ (reasonably up to date)

## Building

Make sure you've built the protocol, as described [here](../proto/README.md)

```zsh
make clean all
```

To format the code (advised before pushing), the following can be ran:
```zsh
make fmt
```

## Running

Assuming there is a `basicQP` running somewhere, you can run the following to run the feature tests. Optionally, you can include a config file and/or an index to be used when connecting.

```
$ ./qp_test [-c config] [-i idx]
```

To close a remote just run:

```
$ ./close_remote [-c config] [-i idx]
```

The defaults in both cases are `../QP/default.conf` for the config and 0 for the index.


## Logging

Change `log_level` in `log.h` to achieve different logging levels.

## Running benchmarks

Each datapoint in the benchmark should be in a fresh QP. To do that you can run the QP perpetually, like so:

```zsh
$ while true; do ./basicQP <config file> idx; sleep .5; done;
```

This way, after a test ends, the client library closes the remote which then spawns again.

### Latency

To run the latency benchmarks, run the following command.

```zsh
$ ./latency_test [-c config file] [-n number of calls] [-o operation] [-p prefix]
```

Concretely, let's imagine we want to benchmark the `sum` call.

```zsh
$ ./latency_test -c ../noQP/default.conf -n 5000 -o sum -p small_vec_
```

This will run the benchmark with 5000 repetitions of the `sum` operation, using the config file `../noQP/default.conf` and prefixing messages to the log with `small_vec_`. This prefixing has proven useful in some situation, but is best generally ignored.

Let's look at a complete example:

```zsh
$ ./latency_test -n 5000 -o sum > results/qp.prof
$ ./latency_test -n 5000 -o ping >> results/qp.prof
$ ./latency_test -c ../noQP/default.conf -n 5000 -o sum > results/no_qp.prof
$ ./latency_test -c ../noQP/default.conf -n 5000 -o ping >> results/no_qp.prof
```

We now have two files, `qp.prof` and `no_qp.prof`.
We can use `boxstats.py` to generate the `gnuplot` script that plots the information.

```zsh
$ cd results
$ ../../../plot_utils/boxstats.py my_plot qp.prof no_qp.prof
$ gnuplot my_plot.gnu
```

You can inclusively edit the gnuplot script and the `dat` file (useful to alter labels). We obtain the following plot.

![](.figs/lat_plot.png)

### Throughput

To run the throughput benchmarks, run the following command.

```zsh
$ ./throughput_test [-c config file] [-d duration] [-l load] [-o operation] [-t tick duration] [-w warmup duration]
```

Concretely, let's imagine we want to benchmark the `sum` call.

```zsh
$ ./throughput_test -d 10 -l 5000 -o sum
```

The tick duration can be tuned, but this is not advised. It can be dynamically calculated by the test.

Let's try to benchmark the throughput of the sum operation on the QP.
Remember to run the QP perpetually before.

```zsh
$ for load in 1000 2500 5000 7500 10000 15000 20000 30000 40000 5000; do ./throughput_test -o sum -l ${load} > results/qp_${load}.prof; sleep 2; done
```

We can now cat them all together and produce a plot with `plotter.py`:

```zsh
$ cd results
$ cat qp_*.prof > qp.prof
$ ../../../plot_utils/plot.py qp.prof
```

There will be a new dir called `qp` with three subdirs: `gnu` (contains the gnuplot scripts), `dat` contains the dat files and `png` contains the images.
There will be two files in the `png` dir: `full_throughput.png` and `latency.png`. There is the possibility of generating more plots (eg: throughput over time, throughput per operation, latency per operation, etc...) but enabling these by default causes the script to become very slow and is as such not advised. The images will look like this (note that the system was never overloaded).

![](.figs/full_throughput.png)
![](.figs/latency.png)

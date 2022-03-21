# basicQP

This is a simple QP, intended to be adapted into other SGX enclaves.

## Structure

There are four main components:
  * [proto](./proto): the protocol interface (specified with [flatbuffers](https://google.github.io/flatbuffers/))
  * [clnt](./clnt): the client library and tests (both functionality and latency) for the QP
  * [QP](./QP): the SGX application for the QP
  * [noQP](./noQP): a non-SGX equivalent of the QP

Each has a more detailed README file.

## Dependencies

Here is a (non-complete) list of dependencies to build this QP:

  * openssl-devel
  * [flatbuffers](https://github.com/google/flatbuffers): it is recommended to build and install from source
  * g++ and clang++ (reasonably up to date)
  * [linux-sgx](https://github.com/intel/linux-sgx)
  * [linux-sgx-driver](https://github.com/intel/linux-sgx-driver)

## Building

### Protocol

Navigate to the `proto` dir. Run:

```zsh
./clean.sh && ./compile.sh && ./install.sh
```

`clean.sh` removes the generated files; `compile.sh` builds the flatbuffers and `install.sh` copies the generated sources to the relevant dirs.

### Client

Navigate to the `clnt` dir. Run:

```zsh
make clean all
```

To format the code (advised before pushing), the following can be ran:
```zsh
make fmt
```

This uses `clang-format` to format the code.

### noQP

Navigate to the `noQP` dir.
```zsh
make clean all
```

To format the code (advised before pushing), the following can be ran:
```zsh
make fmt
```

### QP

Navigate to the `QP` dir.

First, we build `flatbuffers`.

```zsh
$ cd enclave/flatbuffers
$ cmake . && make -j && ./flattests
```

This should echo at the end `ALL TESTS PASSED`.

Second, we build `OpenSSL`. This is the most difficult place of the build, as it is notoriously instable.

```zsh
$ cd enclave/OpenSSL_SGX
& ./sgx_openssl_setup.sh && make depend && make clean && make -j
```

We can now build our SGX application.

```zsh
$ make clean all
```

As with the client, we can format the code.

```zsh
$ make fmt
```

## Configuring

There are two main configuration points.
The first is the file `QP/enclave/enclave.config.xml`. This has the parameters which are used to configure the enclave. The main thing to notice is the Heap size. This can be tuned if the application is OOMing frequently. However, more heap means that the enclave creation will become more expensive and paging might be required.
The second is the file `QP/default.conf`. This file has a list of `<IP> <port>` pairs, which represent the known QPs in the system. The QP can be extended to try and find these and connect to them when setting up.

The log level can be tuned in the client, the QP app and the QP enclave. In all cases, the file `log.h` has a macro `log_level`.
By default, this macro is `0`, which only prints information (setup, etc.) and errors.
At `1`, normal logs are also printed and at `2` finer logs are printed.
Run tests with `log_level = 0`.

## Running

To run a QP:

```zsh
$ ./basicQP <config file> <idx>
```

Where `config file` is the configuration file (eg: `default.conf`) and `idx` is the line of the config which the QP should interpret as refering to itself (ie: it will try to listen on that specific IP and port).
If the QP is compiled with `make PERF=y`, it includes an intrusive performance mechanism. This allows you to insert measurements inside the enclave.
There are two types of measurements: full and count. Count measurements just count the number of times something happened. Full measurements count the number of times something happened, but every time it does, a `double` can represent some value (typically the latency in *us*) and compute the average, range, and sum.
On exit, the QP prints these measurements. To introduce new measurements, see how files `/app/basicQP.cc`, `/enclave/handlers.cc` implement the measurements. It is important to note that measuring latency inside an enclave involves an OCALL, which is very expensive. Thus, full measurements are designed to be used as a performance debugging mechanism, not to produce final results.

With the QP running, the client test can be run:

```zsh
$ ./qp_clnt
```

To run latency tests, adapt and run, like the sum latency tests:

```zsh
$ ./latency_test [-c config file] [-n number of calls] [-o operation] [-p prefix]
```

To plot the results of your latency experiment, use the script in `/plot_utils`.
Suppose you ran the experiment several times, with slightly different parameters. You can join different experiments in a histogram.

```sum
$ ../../plot_utils boxstats.py sum exp1.prof exp2.prof
```
This will do three things: 1) print statistics per file; 2) generate a `sum.dat` file with the relevant statistics for the histogram and 3) generate a `sum.gnu` file with the `gnuplot` to generate the histogram. Run `gnuplot sum.gnu` to generate a `sum.png` file. It will have a single histogram, labelled `sum` with two columns `exp1` and `exp2`.

To run the throughput tests, do the following:

```zsh
$ ./throughput_test [-c config file] [-d duration] [-l load] [-o operation] [-t tick duration] [-w warmup duration]
```

### Troubleshooting

When running ./basicQP:

```zsh
./basicQP: error while loading shared libraries: libsgx_urts_sim.so: cannot open shared object file: No such file or directory
```
Add to .bashrc:
```zsh
source /opt/intel/sgxsdk/environment
```
Then, run:
```zsh
source .bashrc
```

## Adapting

To adapt the QP to a new functionality, there are three general steps to be taken.
There are comments in the code of the form `XXX: CHANGE ME`. You can grep for this if you are ever lost.

### Adapt the protocol

Adapt the protocol schema in `/proto`. Recompile the schema.

### Adapt the client

Adapt the client code in `/clnt`. Focus on checking whether you need a new result type (probably not) and how to compose flatbuffers requests and decompose flatbuffers responses. It is relatively simple. Adapt the API and the `qlnt_test`. There is no good way of unit testing the QP, so this is the best functionality test you can have. Adapt the latency and throughput tests (it is very simple to do so).

### Adapt the QP

Adapt the QP code in `/QP`. The application can probably remain. In `QP/enclave`, the important file to take in to account is `handlers.cc`. This file has a function `handle_client_message`, which deconstructs and multiplexes client requests. Add your request/adapt existing ones.
Now suppose you are adding a `count` operation. After doing the previous step, copy `sum_handler.h` and `sum_handler.cc` into `count_handler.h` and `count_handler.cc`. Adapt both the API and the implementation accordingly. Remember that the operation handler is responsible for composing the flatbuffer response and sending it (or appending it at the very least).

This should be enough.

#### Updating the noQP with the QP

At this point you may (and should ask) how much work is it to convert the QP to a noQP.
It depends.
If your handlers are relatively simple, it may be trivial.
First, make your changes in the `QP`.
Imagine you want to add a `count` operation.
You adapt the `handlers.cc` and add a `count_handler.cc` and `count_handler.h`, you can copy these files to the `noQP` dir.
In principle this should work. If the handlers are more complex and handle more state, this has to be reflected in the `noQP`.

## Cloning

To clone this QP, simply run `./clone.sh <new shiny name>` to create a new copy, with a different name, of the current QP.

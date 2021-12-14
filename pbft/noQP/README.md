# basicQP: noQP

Non SGX version: useful for baselines

## Dependencies

Here is a (non-complete) list of dependencies to build this QP:

  * openssl-devel
  * [flatbuffers](https://github.com/google/flatbuffers): it is recommended to build and install from source
  * g++ and clang++ (reasonably up to date)

## Building

Make sure you've built the protocol, as described [here](../proto)

```zsh
make clean all
```

To format the code (advised before pushing), the following can be ran:
```zsh
make fmt
```

## Running

To run a QP:

```zsh
$ ./basicQP <config file> <idx>
```

Where `config file` is the configuration file (eg: `default.conf`) and `idx` is the line of the config which the QP should interpret as refering to itself (ie: it will try to listen on that specific IP and port).
If the QP is compiled with `make PERF=y`, it includes an intrusive performance mechanism. This allows you to insert measurements inside the enclave.
There are two types of measurements: full and count. Count measurements just count the number of times something happened. Full measurements count the number of times something happened, but every time it does, a `double` can represent some value (typically the latency in *us*) and compute the average, range, and sum.
On exit, the QP prints these measurements. To introduce new measurements, see how files `/app/basicQP.cc`, `/enclave/handlers.cc` implement the measurements. It is important to note that measuring latency inside an enclave involves an OCALL, which is very expensive. Thus, full measurements are designed to be used as a performance debugging mechanism, not to produce final results.

You can also run the QP perpetually like this:

```zsh
$ while true; do ./basicQP <config file> idx; sleep .5; done;
```

This is *extremely* useful for performance testing. After each profile test, the client closes the remote server, which then promptly comes back to live for another round.

## Configuration

The QP accepts a configuration file and an index. The config file must be a sequence of lines of the form `<IP addr> <port>` and the index should be a position of the node the QP should assume the role of in that configuration file.

## Logging

Change `log_level` in `log.h` to achieve different logging levels.

## Converting from a QP

At this point you may (and should ask) how much work is it to convert the QP to a noQP.
It depends.
If your handlers are relatively simple it may be trivial.
First, make your changes in the `QP`.
Imagine you want to add a `count` operation.
You adapt the `handlers.cc` and add a `count_handler.cc` and `count_handler.h`, you can copy these files to the `noQP` dir.
In principle this should work. If the handlers are more complex and handle more state, this has to be reflected in the `noQP`.

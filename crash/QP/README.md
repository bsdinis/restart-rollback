# basicQP: QP

SGX version

## Dependencies

Here is a (non-complete) list of dependencies to build this QP:

  * g++ and clang++ (reasonably up to date)
  * [linux-sgx](https://github.com/intel/linux-sgx)
  * [linux-sgx-driver](https://github.com/intel/linux-sgx-driver)

Make sure you source the SGX environment. Otherwise the code won't build/link.

## Building


First, we build `flatbuffers`.

```zsh
$ cd enclave/flatbuffers
$ cmake . && make -j && ./flattests
```
This should echo at the end `ALL TESTS PASSED`.

```zsh
make clean all
```

Second, we build `OpenSSL`. This is the most difficult place of the build, as it is notoriously instable.

```zsh
$ cd enclave/OpenSSL_SGX
& ./sgx_openssl_setup.sh && make depend && make clean && make -j
```

We can now build our SGX application.

```zsh
$ make clean all
```

To format the code (advised before pushing), the following can be run:
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

Change `log_level` in `app/log.h` and in `enclave/log.h` to achieve different logging levels.

## Troubleshooting

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

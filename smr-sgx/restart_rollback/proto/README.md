# basicQP: protocol

Protocol, specified in flatbuffers

## Dependencies

Here is a (non-complete) list of dependencies to build the protocol:

  * [flatbuffers](https://github.com/google/flatbuffers): it is recommended to build and install from source
  * g++ and clang++ (reasonably up to date)

## Building

```zsh
$ ./clean.sh && ./compile.sh && ./install.sh
```

## Changing the protocol

To change the protocol, follow the `XXX: CHANGE ME` marks. You will need to add a new request type, new argument type and (maybe) a new response type.

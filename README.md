# gas-cli

[![Build](https://github.com/lobis/gas-generator/actions/workflows/build.yml/badge.svg)](https://github.com/lobis/gas-generator/actions/workflows/build.yml)

`gas-cli` is a command line tool for generating, merging or
reading [Garfield++](https://gitlab.cern.ch/garfield/garfieldpp) gas files.

## Installation

[ROOT](https://github.com/root-project/root) is required as it is a dependency of Garfield itself even though it's not
really needed for this project (TODO: remove ROOT dependency).

Garfield is not a required dependency as the CMake configuration will build it from source if its not found on the
system. This is not practical for ROOT as it's a very large dependency.

Use CMake to build and install the project (run commands at the root of the project):

```
cmake -S . -B build
cmake --build build
cmake --install build
```

## Command Line Interface

A full list of commands can be retrieved via

```gas-cli --help```

### Generating a gas file

A gas file can be generated from the command line parameters using the `generate` subcommand.

```
gas-cli generate --components C4H10 2.3 Ar -o /tmp/test.gas --electric-field-log 1 1000 10
```

multiple electric field configurations (linear, log, points) can be defined at once.
Close points are discarded according to predefined tolerances. The command below will produce 200 entries.

```
gas-cli generate --components Ar 48.85 Xe 48.85 C4H10 --pressure 1.0 --efield-lin 0 1000 110 --efield-log 0.1 1000 107 --collisions 10
```

### Reading a gas file

A gas file can be read and a json containing some useful gas properties can be generated using the `read` subcommand.

```
gas-cli read -i input.gas -o output.json
```

### Merging multiple gas files

Multiple gas files can be combined into one using the `merge` subcommand.

```
gas-cli merge -i file1.gas file2.gas file3.gas -o merge.gas
```

## Docker image

A docker image is available as a [GitHub package](https://github.com/lobis/gas-generator/pkgs/container/gas-cli).

```
docker run --rm --pull=always ghcr.io/lobis/gas-cli --help
```

To run a gas simulation and save it to a local directory you can use the following command:

```
docker run -d --rm --pull=always --mount type=bind,source=/tmp/gases,target=/output ghcr.io/lobis/gas-cli generate --components Ar 48.85 Xe 48.85 C4H10 --dir /output --pressure 1.0 --efield-lin 0 1000 110 --efield-log 0.1 1000 107
```

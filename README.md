# HDLC library - libhdlc

## Introduction

High-Level Data Link Control (HDLC) is a communication protocol used for transmitting data between devices in telecommunication and networking. Developed by the International Organization for Standardization (ISO), it is defined in the standard ISO/IEC 13239:2002. See[Wikipedia](https://en.wikipedia.org/wiki/High-Level_Data_Link_Control) for more information.

`libhdlc` is a small code-size C implementation of the HDLC communication protocol. The implementation has a low memory footprint, which makes it especially suitable for memory restricted microcontrollers (e.g., Cortex-M0+).

## Requirements

The following packages should be installed on the build system:

- Make
- CMake (version >=3.20)
- Ninja
- GCC

## Build

You can build the [library](./lib), [examples](./examples) and [tests](./tests) using the following command:

```sh
make
```

The `make` command will build the [library](./lib), [examples](./examples) and [tests](./tests). The default build type is `Release`. By adding a `BUILD_TYPE` prefix with value `Debug`, `make` will use `Debug` as the build type:

```sh
BUILD_TYPE=Debug make
```

## Install

The following command will install the [library](./lib), [examples](./examples) and [tests](./tests):

```sh
make install
```

The `make` command will install the [library](./lib), [examples](./examples) and [tests](./tests). By default, the `Release` artifacts will be installed. By adding a `BUILD_TYPE` prefix with value `Debug`, `make` will install the `Debug` artifacts:

```sh
BUILD_TYPE=Debug make install
```

By default, all the components will be installed in a `bin` folder in the root of the repository. In this folder a subfolder is created, the name of the subfolder depends on the build type (i.e., `Release` or `Debug`). The [library](./lib), [examples](./examples) and [tests](./tests) will be installed in separate subfolders, `lib`, `examples` and `tests`, respectively.

## Running examples

By default, the examples are installed in `bin/<build-type>/examples`. To run the [basic example](./examples/basic/src/main.c), run the following executable:

```sh
./bin/Release/examples/basic
```

To run the `Debug` example, replace `Release` with `Debug`.

## Running tests

By default, the tests are installed in `bin/<build-type>/tests`. To run the [tests](./tests/src/main.cpp), run the following executable:

```sh
./bin/Release/tests/tests
```

To run the `Debug` tests, replace `Release` with `Debug`.

## Cleanup

To clean up all the build and install files, use the following command:

```sh
make clean
```

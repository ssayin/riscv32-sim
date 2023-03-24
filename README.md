<!--
SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>

SPDX-License-Identifier: CC-BY-SA-4.0
-->

![](https://github.com/ssayin/riscv32-sim/actions/workflows/build.yml/badge.svg)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=coverage)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=sqale_index)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)

<!--
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=bugs)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
-->

riscv32-sim 
===========

<p align="center" width="100%">
  <img width="60%" src="misc/images/screenshot.svg">
</p>

An easy-to-use, still-in-development RISC-V 32-bit simulator.

<!--toc:start-->
- [riscv32-sim 
===========
](#riscv32-sim)
  - [Getting Started](#getting-started)
    - [Build Requirements](#build-requirements)
    - [Build Flags](#build-flags)
    - [Building on Linux](#building-on-linux)
    - [Building and Running Tests [Optional]](#building-and-running-tests-optional)
  - [Using](#using)
    - [CLI Options and Flags](#cli-options-and-flags)
    - [How does the simulator determine when to halt?](#how-does-the-simulator-determine-when-to-halt)
  - [Built With](#built-with)
<!--toc:end-->

## Getting Started

### Build Requirements

- [git](https://git-scm.com/downloads)
- [cmake](https://cmake.org/download/) >= 3.23.2
- [a cmake supported native build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7))
- [a compiler with C++20 support](https://en.cppreference.com/w/cpp/compiler_support) - required
  features: [consteval, concepts]

### Build Flags

`-DBUILD_TESTING=[ON/OFF]`: Enabled by default.

~~`-DENABLE_TCP=[ON/OFF]`: Enable TCP/IP for handling external interrupts. Only UNIX-like platforms are currently supported. Only on Linux has it been tested. Not fully integrated. This feature will be available in the first release.~~

`-DBUILD_RUNTIME=[ON/OFF]`: Cross-compile the programs in the `runtime` directory and add them as test targets. Before you enable this flag, ensure that the GCC RISC-V toolchain is in your `PATH` and that the `RISCV` variable is set to the compiler root directory.

`-DISA_TESTS_DIR=[PATH]`: Location of [riscv-tests](https://github.com/riscv-software-src/riscv-tests) **binaries**. 

`-DCOVERAGE=[ON/OFF]`: For GCC only. Compile with `-fprofile-arcs -ftest-coverage` and link `gcov` library.

### Building on Linux

Fire up your favorite terminal and run those programs in shell:

```sh
# clone the repo with submodules
git clone --recurse-submodules https://github.com/ssayin/riscv32-sim.git

# cd to local repo
cd riscv32-sim

# update submodules
git submodule update --init --recursive

# configure and generate
# see cmake documentation for other generators
# and how to configure them
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build

# build using GNU Make
make -C build
```

The executable binary will end up in ``build/riscv32-sim`` unless you specify a custom binary directory during
configuration.

*I do not recommend installing the binary in your system path.*

### Building and Running Tests [Optional]

## Using

```sh
# with the executable in working directory
./riscv32-sim <your_elf_binary>
```

### CLI Options and Flags

**<<<ATTENTION>>>** 

Since exporting hart state is a recently introduced feature, the **serialization format** is quite likely to **change**.

---

`-d,--dump` Dump ELF header then exit.

`--step` Enable manual step.

`--trace` Enable disasm trace, logged to `trace.log`.

`--export` Export hart state as JSON.

`--tohost <sym>` Customize the tohost symbol. In order to use your own __start() and __exit(int) routines, you can tell
the simulator to use this symbol instead. Be sure to define it in your linker script.

---

The options below have not been thoroughly tested and may cause your computer to malfunction. **Caution is advised.**

`--timer` Enable machine timer interrupts.

* `--mtime` Address of mtime in memory.

* `--mtimecmp` Address of mtimecmp in memory.

* `--interval` Timer interval.

### How does the simulator determine when to halt?

The simulator can be instructed to stop in the following ways:

1. Create a symbol in your linker script called "tohost." If this is something you have never done before, you can look
   at the documentation on this website: https://doc.ecoscentric.com/gnutools/doc/ld/Scripts.html#Scripts. Writing
   to `tohost` terminates the program.

2. Declare a function with *\_\_naked\_\_* attribute. In function definition, set `register a7 (x17)` to `93`.
   Then `ecall`. Call this function in your `__exit(int)` routine. You can also set the other argument
   registers `a0, a1, a2, a3, a4, a5, a6`. The value in the `a0` register will be reported as the exit code. For more
   information on how the linux kernel implements syscalls,
   visit https://github.com/torvalds/linux/blob/master/arch/riscv/kernel/sbi.c.

**Note:** `syscall(93)` is handled by the simulator despite SBI not yet being implemented.

## Built With

* [CLI11](https://github.com/CLIUtils/CLI11) - CLI11 is a command line parser for C++11 and beyond that provides a rich
  feature set with a simple and intuitive interface.
* [ELFIO](https://github.com/serge1/ELFIO) - A header-only C++ library intended for reading and generating files in the
  ELF binary format.
* [{fmt}](https://github.com/fmtlib/fmt) - An open-source formatting library providing a fast and safe alternative to C
  stdio and C++ iostreams.
* [json](https://github.com/nlohmann/json) - JSON for Modern C++
* [riscv-disassembler](https://github.com/michaeljclark/riscv-disassembler) - RISC-V Disassembler with support for
  RV32/RV64/RV128 IMAFDC

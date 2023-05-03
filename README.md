<!--
SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>

SPDX-License-Identifier: CC-BY-SA-4.0
-->

![](https://github.com/ssayin/riscv32-sim/actions/workflows/build.yml/badge.svg)
![](https://github.com/ssayin/riscv32-sim/actions/workflows/build-windows.yml/badge.svg)
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

# riscv32-sim 

<p align="center" width="100%">
  <img width="60%" src="misc/images/screenshot.svg">
</p>

An easy-to-use, still-in-development RISC-V 32-bit instruction-accurate simulator.

## Table of Contents

<!--toc:start-->
- [riscv32-sim](#riscv32-sim)
  - [Table of Contents](#table-of-contents)
  - [Why does this project exist?](#why-does-this-project-exist)
  - [Features](#features)
  - [Roadmap](#roadmap)
    - [In Progress](#in-progress)
    - [Planned](#planned)
  - [Directory Structure](#directory-structure)
  - [Getting Started](#getting-started)
    - [Build Requirements](#build-requirements)
    - [Build Flags](#build-flags)
    - [Building on Linux](#building-on-linux)
    - [Building and Running Tests [Optional]](#building-and-running-tests-optional)
  - [Using](#using)
    - [CLI Options and Flags](#cli-options-and-flags)
    - [How does the simulator determine when to halt?](#how-does-the-simulator-determine-when-to-halt)
  - [Troubleshooting](#troubleshooting)
    - [cc1: error: '-march=rv32im_zicsr': unsupported ISA subset 'z'](#cc1-error-marchrv32imzicsr-unsupported-isa-subset-z)
    - [/usr/lib/gcc/riscv64-unknown-elf/10.2.0/include/stdint.h:9:16: fatal error: stdint.h: No such file or directory 9 | # include_next &lt;stdint.h&gt;](#usrlibgccriscv64-unknown-elf1020includestdinth916-fatal-error-stdinth-no-such-file-or-directory-9-includenext-ltstdinthgt)
    - [ABI is incompatible with that of the selected emulation: target emulation 'elf64-littleriscv' does not match 'elf32-littleriscv'](#abi-is-incompatible-with-that-of-the-selected-emulation-target-emulation-elf64-littleriscv-does-not-match-elf32-littleriscv)
  - [Built With](#built-with)
  - [Author](#author)
  - [License (Updated: 2023-04-11)](#license-updated-2023-04-11)
    - [Regents of the University of California License](#regents-of-the-university-of-california-license)
<!--toc:end-->

## Why does this project exist?

The processor simulator is a lightweight alternative to existing IA (Instruction-accurate) processor simulators. Designed for the 32-bit RISC-V instruction set architecture and its extensions, it offers gentle learning curve, and optimizes performance. This also allows for simplified code base, faster simulation, accurate results, and increased productivity and efficiency in the development process.

## Features

- Supports the RV32I, RV32M, and RVC instruction set extensions

- Supports machine timer interrupts

- Supports CSRs (basic).

- Supports M-mode

- Exports hart-state in JSON format

- Exports disassembly trace

## Roadmap

### In Progress

- Implement WPRI, WLRL, WARL fields

- TCP/IP-based external interrupts

- GDB stub, attach/step, EBREAK

- File and network I/O using ECALL

### Planned

- Toggle instructions in misa register

- GPIO

- UART

- Ability to configure enabled/disabled extensions in CLI

- Implement extension A (atomic instructions)

- Implement extension F (single-precision floating-point)

- Implement base extension E (embedded)

- CSR field modulation (I and E)

## Directory Structure

```
├── cmake
├── external
├── LICENSES
├── misc
│   └── images
├── riscv32-sim
│   ├── include
│   │   ├── iss
│   │   ├── memory
│   │   └── zicsr
│   ├── lib
│   │   ├── riscv32-decoder
│   │   │   ├── decoder
│   │   │   │   ├── include
│   │   │   │   │   └── decoder
│   │   │   │   └── src
│   │   │   └── LICENSES
│   │   └── riscv32-sim-common
│   │       └── include
│   │           └── common
│   ├── src
│   │   ├── iss
│   │   └── memory
│   └── unittest
├── runtime
│   ├── arch-test
│   ├── basic
│   │   ├── cmake
│   │   ├── crt
│   │   └── src
│   ├── riscv-tests
│   └── tvm
└── tools
```
- `cmake`: Contains CMake related scripts and configuration files.
- `external`: Houses external dependencies and libraries.
- `LICENSES`: Contains the licenses for the project and its dependencies.
- `misc`: Miscellaneous files, including images.
    - `images`: Image files used in the project documentation.
- `riscv32-sim`: Main project source code and library.
    - `include`: Header files for the project, organized into subdirectories for different modules.
    - `lib`: Libraries used in the project.
        - `riscv32-decoder`: RISC-V instruction decoding library.
        - `riscv32-sim-common`: Shared data structures and utilities for decoding RISC-V instructions.
    - `src`: Source files for the project, organized into subdirectories for different modules.
- `unittest`: Unit tests for the project.
- `runtime`: Contains runtime support files, test programs, and test automation scripts for the simulator to generate coverage.
    - `arch-test`: Custom setup and riscv-arch-test suite integration for simulator verification.
    - `basic`: C programs and runtime support files for testing the simulator.
        - `cmake`: CMake scripts for building and testing.
        - `crt`: C runtime support files and linker scripts.
        - `src`: Source files for C programs used in testing.
    - `riscv-tests`: Includes a CMakeLists.txt file to locate available riscv-tests \*.elf files. For information on building these files within the project, check out the relevant [build.yml](https://github.com/ssayin/riscv32-sim/blob/f96c8d6cf97b554234b3e5915be838faad382802/.github/workflows/build.yml#L88) section. You can also visit [riscv-tests](https://github.com/riscv-software-src/riscv-tests) for further details.
    - `tvm`: Hosts RISC-V 32-bit test programs and automation scripts, leveraging [riscv-test-env](https://github.com/riscv/riscv-test-env) to execute tests on the simulator.
- `tools`: Additional tools and script utilities used in the project.

## Getting Started

### Build Requirements

- [git](https://git-scm.com/downloads)
- [cmake](https://cmake.org/download/) >= 3.23.2
- [a cmake supported native build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7))
- [a compiler with C++20 support](https://en.cppreference.com/w/cpp/compiler_support) - required
  features: [consteval, concepts]

### Build Flags

`-DBUILD_TESTING=[ON/OFF]`: Enable/disable building tests (default: ON).

`-DBUILD_RUNTIME=[ON/OFF]`: Cross-compile programs in the runtime directory and add them as test targets. Requires GCC RISC-V toolchain in your PATH and the RISCV variable set to the compiler root directory.

`-DISA_TESTS_DIR=[PATH]`: Specify the path to riscv-tests binaries.

`-DCOVERAGE=[ON/OFF]`: (GCC only) Compile with coverage flags and link gcov library.

`-DTEST_EXPORT_JSON_HART_STATE=[ON/OFF]`: Enable/disable exporting hart state as JSON (default: OFF).

`-DTEST_EXPORT_TRACE_DISASSEMBLY=[ON/OFF]`: Enable/disable exporting disassembly trace (default: OFF).

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

*I do not recommend installing the binary in your system path.*

### Building and Running Tests [Optional]

See [build.sh](tools/build.sh).

## Using

```sh
# with the executable in working directory
./riscv32-sim <your_elf_binary>
```

### CLI Options and Flags

**Note:** The serialization format for exporting hart state may change in the future.

| Flag                              | Description                                                                                                                     |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| –dump                             | Dump ELF header and exit                                                                                                        |
| –step                             | Enable Manual step mode                                                                                                         |
| -j&lt;filename&gt;,–json-output&lt;filename&gt;  | Set JSON output (required for hart state export)                                                                                |
| -d&lt;filename&gt;,–disas-output&lt;filename&gt; | Set disassembly trace output (required for trace export)                                                                        |
| –tohost &lt;sym&gt;               | Customize the tohost symbol. Useful for custom `__start()` and `__exit(int)` routines. Define the symbol in your linker script. |

**Caution**: The following options are experimental and may cause your computer to malfunction.

- **--timer**: Enable machine timer interrupts.
    - **--mtime &lt;address&gt;**: Set mtime memory address.
    - **--mtimecmp &lt;address&gt;**: Set mtimecmp memory address.
    - **--interval &lt;value&gt;**: Set timer interval.

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

## Troubleshooting

### cc1: error: '-march=rv32im_zicsr': unsupported ISA subset 'z'

Check the relevant issues on riscv-gcc repository
1. https://github.com/riscv-collab/riscv-gcc/issues/339
2. https://github.com/riscv-collab/riscv-gnu-toolchain/issues/1053

Check the upstream patch:
https://github.com/riscv-collab/riscv-gnu-toolchain/pull/1054

**UPDATE:** On a second thought, you can set up your toolchain by setting -DCMAKE\_TOOLCHAIN\_FILE; build flag. This may help bypassing some issues related to incompatible ABIs. However, I cannot guarantee its effectiveness in all cases.

Example: `cmake -DCMAKE_TOOLCHAIN_FILE=my_toolchain_file.cmake`.

Or you can edit [riscv.cmake](cmake/riscv.cmake) and [arch-test/CMakeLists.txt](runtime/arch-test/CMakeLists.txt) according to your needs.

Please keep in mind that this simulator only supports the 20191213 RISC-V specification.

### /usr/lib/gcc/riscv64-unknown-elf/10.2.0/include/stdint.h:9:16: fatal error: stdint.h: No such file or directory 9 | # include_next &lt;stdint.h&gt;

Please refer to: [ABI is incompatible with that of the selected emulation: target emulation 'elf64-littleriscv' does not match 'elf32-littleriscv'](#abi-is-incompatible-with-that-of-the-selected-emulation-target-emulation-elf64-littleriscv-does-not-match-elf32-littleriscv) 

### ABI is incompatible with that of the selected emulation: target emulation 'elf64-littleriscv' does not match 'elf32-littleriscv'

If you encounter the error when building tests, try installing GCC binutils for RISC-V 32-bit. 

However, if you're using Ubuntu LTS 20.04 or 22.04, unfortunately, the binutils for riscv32 is not available in the official repositories. 

In that case, you can either compile it yourself or use Embecosm build which is used in the GitHub Actions build workflow. Check the relevant section of the [build.yml](.github/workflows/build.yml) file for extracting and setting up PATH on Linux.

If you're using Arch Linux, you can install binutils-riscv32 from the AUR.

## Built With

* [CLI11](https://github.com/CLIUtils/CLI11) - CLI11 is a command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface.

* [ELFIO](https://github.com/serge1/ELFIO) - A header-only C++ library intended for reading and generating files in the ELF binary format.

* [json](https://github.com/nlohmann/json) - JSON for Modern C++

* [riscv-disassembler](https://github.com/michaeljclark/riscv-disassembler) - RISC-V Disassembler with support for RV32/RV64/RV128 IMAFDC

* [spdlog](https://github.com/gabime/spdlog) - Very fast, header-only/compiled, C++ logging library.

## Author

**Serdar Sayın** - *Initial work, project maintainer* - [GitHub](https://github.com/ssayin)

If you have any questions or suggestions, feel free to contact me at [serdarsayin@pm.me](mailto:serdarsayin@pm.me).

## License (Updated: 2023-04-11)

This project has multiple licenses:

- The source code and build scripts are licensed under the [MIT License](LICENSES/MIT.txt).
- Code borrowed from the Spike simulator "arith.hpp" is licensed under the Regents of the University of California License (see below).
- All documentation is licensed under [Creative Commons Attribution-ShareAlike 4.0 International License (CC-BY-SA-4.0)](LICENSES/CC-BY-SA-4.0.txt).
- Some configuration and data files are licensed under [Creative Commons Zero v1.0 Universal (CC0-1.0)](LICENSES/CC0-1.0.txt).

**Note:** For more accurate licensing information, please check the individual files, as keeping this section up to date can be challenging.

### Regents of the University of California License

Copyright (c) 2010-2017, The Regents of the University of California
(Regents).  All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the Regents nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

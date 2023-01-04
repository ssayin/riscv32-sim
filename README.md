riscv32-sim ![](https://github.com/ssayin/riscv32-sim/actions/workflows/build.yml/badge.svg) [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=ssayin_riscv32-sim&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=ssayin_riscv32-sim)
===========


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

<p align="center" width="100%">
  <img width="60%" src="misc/images/screenshot.svg">
</p>

An easy-to-use, still-in-development RISC-V 32-bit simulator.

## Getting Started

### Build Requirements

- a [compiler with C++20 support](https://en.cppreference.com/w/cpp/compiler_support) - required features: [consteval, concepts]
- [git](https://git-scm.com/downloads)
- [cmake](https://cmake.org/download/) >= 3.23.2
- a [cmake supported native build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7))
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
The executable binary will end up in ``build/riscv32-sim`` unless you specify a custom binary directory during configuration.

*I do not recommend installing the binary in your system path.*

### Building and Running Tests [Optional]

## Using

```sh
# with the executable in your path (either local or ENV path)
riscv32-sim <your_elf_binary>
```

## Built With

* [ELFIO](https://github.com/serge1/ELFIO) - A header-only C++ library intended for reading and generating files in the ELF binary format.
* [{fmt}](https://github.com/fmtlib/fmt) - An open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.

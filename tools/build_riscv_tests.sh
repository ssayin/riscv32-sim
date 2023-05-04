#!/usr/bin/bash -e

# SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

git clone --recurse-submodules https://github.com/riscv-software-src/riscv-tests.git
cd riscv-tests
git submodule update --init --recursive

# Patch Makefile.in
git clone https://gist.github.com/b5c1b033143cbdce51afa0c1280970a8.git
patch < b5c1b033143cbdce51afa0c1280970a8/patch1.diff

# Configure and Make
autoupdate
autoconf
mkdir build
cd build
../configure --prefix=$RISCV --with-xlen=32
make install

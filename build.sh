#!/usr/bin/bash -e

# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

BUILD_DIR=build
ISA_TESTS_DIR=/opt/riscv32

cmake -B $BUILD_DIR -GNinja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCOVERAGE=ON \
  -DISA_TESTS_DIR=$ISA_TESTS_DIR \
  -DENABLE_TCP=ON \
  -DBUILD_RUNTIME=ON \
  #-DBUILD_TESTING=OFF

ninja -v -C $BUILD_DIR

ctest --test-dir $BUILD_DIR --output-on-failure

gcovr -e external -e build 

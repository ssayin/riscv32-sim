#!/usr/bin/bash -e

# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

if [ $# -ne 3 ]; then
  echo "Usage: $0 <BUILD_DIR> <BUILD_TYPE> <ISA_TESTS_DIR>"
  exit 1
fi

BUILD_DIR=$1
BUILD_TYPE=$2
ISA_TESTS_DIR=$3

cmake -B $BUILD_DIR -GNinja \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCOVERAGE=ON \
  -DISA_TESTS_DIR=$ISA_TESTS_DIR \
  -DENABLE_TCP=ON \
  -DBUILD_RUNTIME=ON \
  -DTEST_EXPORT_JSON_HART_STATE=ON \
  -DENABLE_UBENCHMARKS=ON
  #-DBUILD_TESTING=OFF

ninja -v -C $BUILD_DIR

ctest --test-dir $BUILD_DIR --output-on-failure

gcovr -e external -e $BUILD_DIR

gcovr -e external -e $BUILD_DIR --sonarqube > coverage.xml

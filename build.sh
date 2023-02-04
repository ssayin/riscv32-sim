#!/usr/bin/bash -e

BUILD_DIR=build

cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_RUNTIME=ON -DCOVERAGE=ON -DISA_TESTS_DIR=/opt/riscv32 -GNinja
ninja -v -C $BUILD_DIR
ctest --test-dir $BUILD_DIR --output-on-failure
gcovr -e external -e build 

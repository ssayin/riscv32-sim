#!/usr/bin/bash

cmake -B build -DBUILD_TESTS=ON -DISA_TESTS_DIR=/opt/riscv32 -GNinja
ninja -C build
ctest --test-dir build/test/ --output-on-failure
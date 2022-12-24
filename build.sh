#!/usr/bin/bash

cmake -B build -DBUILD_TESTS=ON -GNinja
ninja -C build
ctest --test-dir build/test/ --output-on-failure

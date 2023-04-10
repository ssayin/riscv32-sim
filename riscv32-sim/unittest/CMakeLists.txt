# SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

add_unit_test(sparse_memory_test sparse_memory_test.cc
              ${PROJECT_SOURCE_DIR}/riscv32-sim/src/memory/sparse_memory.cpp)

add_unit_test(json_test json_test.cc)

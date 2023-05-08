# SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

find_package(benchmark REQUIRED)

add_executable(alu_bench alu_bench.cpp alu.cpp)
target_link_libraries(alu_bench benchmark::benchmark riscv32-sim-common)
target_compile_options(alu_bench PRIVATE "-O3")

add_custom_command(
  TARGET alu_bench
  POST_BUILD
  COMMAND ${CMAKE_OBJDUMP} -d $<TARGET_FILE:alu_bench> > alu_bench.dump
  COMMENT "Generating objdump output for alu_bench"
  VERBATIM)

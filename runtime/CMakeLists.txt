# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

macro(add_riscv32sim_test_target TARGET)
  set(LOG_FILE_BASE ${RISCV32_SIM_TRACE_OUTPUT_DIRECTORY}/${TARGET})
  set(EXPORT_JSON "-j${LOG_FILE_BASE}.json")
  set(EXPORT_DISAS_TRACE "-d${LOG_FILE_BASE}.log")

  add_test(
    NAME ${TARGET}
    COMMAND
      $<TARGET_FILE:riscv32-sim> $<TARGET_FILE:${TARGET}>
      $<$<BOOL:${TEST_EXPORT_JSON_HART_STATE}>:${EXPORT_JSON}>
      $<$<BOOL:${TEST_EXPORT_TRACE_DISASSEMBLY}>:${EXPORT_DISAS_TRACE}>)
endmacro()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/runtime)

add_subdirectory(basic)
add_subdirectory(tvm)
add_subdirectory(riscv-tests)
add_subdirectory(arch-test)

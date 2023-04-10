# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

set(ISA_TEST_GLOB "rv32u[imc]-p-*")

if(NOT DEFINED ISA_TESTS_DIR)
  message(STATUS "ISA_TESTS_DIR was not set")

  if(NOT DEFINED $ENV{RISCV})
    message(STATUS "RISCV environment variable was not set")
    message(STATUS "Skip adding riscv-tests")
  else()
    set(ISA_TESTS_DIR
        "$ENV{RISCV}/share/riscv-tests/isa"
        CACHE PATH "Set riscv-tests build PATH")

    message(STATUS "Set ISA_TESTS_DIR to ${ISA_TESTS_DIR}")
  endif()
endif()

if(DEFINED ISA_TESTS_DIR)
  file(GLOB_RECURSE ISA_TESTS ${ISA_TESTS_DIR}/${ISA_TEST_GLOB})

  # Exclude fence_i as it is not yet implemented
  list(FILTER ISA_TESTS EXCLUDE REGEX ".*rv32ui-p-fence_i.*")

  list(FILTER ISA_TESTS EXCLUDE REGEX ".*.dump$")

  if(ISA_TESTS)
    foreach(ISA_TEST ${ISA_TESTS})
      get_filename_component(FILENAME_BASE ${ISA_TEST} NAME_WLE)
      set(LOG_FILE_BASE ${RISCV32_SIM_TRACE_OUTPUT_DIRECTORY}/${FILENAME_BASE})
      set(EXPORT_JSON "-j${LOG_FILE_BASE}.json")
      set(EXPORT_DISAS_TRACE "-d${LOG_FILE_BASE}.log")

      add_test(
        NAME ${ISA_TEST}
        COMMAND
          $<TARGET_FILE:riscv32-sim> ${ISA_TEST}
          $<$<BOOL:${TEST_EXPORT_JSON_HART_STATE}>:${EXPORT_JSON}>
          $<$<BOOL:${TEST_EXPORT_TRACE_DISASSEMBLY}>:${EXPORT_DISAS_TRACE}>)

      message(STATUS "Add test target ${ISA_TEST}")
      set_tests_properties(${ISA_TEST} PROPERTIES TIMEOUT 10)
      set_tests_properties(${ISA_TEST} PROPERTIES PASS_REGULAR_EXPRESSION
                                                  "Exited with 0x0")
    endforeach()
  elseif()
    message(STATUS "No riscv-tests found in ${ISA_TESTS_DIR}")
  endif()
endif()

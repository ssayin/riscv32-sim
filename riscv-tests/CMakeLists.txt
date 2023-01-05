set(ISA_TEST_GLOB "rv32u[im]-p-*")

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

  list(FILTER ISA_TESTS EXCLUDE REGEX ".*.dump$")

  if(ISA_TESTS)
    foreach(ISA_TEST ${ISA_TESTS})
      add_test(NAME ${ISA_TEST} COMMAND $<TARGET_FILE:riscv32-sim> ${ISA_TEST})
      message(STATUS "Add test target ${ISA_TEST}")
      set_tests_properties(${ISA_TEST} PROPERTIES PASS_REGULAR_EXPRESSION
                                                  "Exited with 0x0")
    endforeach()
  elseif()
    message(STATUS "No riscv-tests found in ${ISA_TESTS_DIR}")
  endif()
endif()

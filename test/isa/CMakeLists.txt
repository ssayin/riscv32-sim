add_subdirectory(sim)
add_subdirectory(tvm)

if(NOT DEFINED ISA_TESTS_DIR)
  message(STATUS "ISA_TESTS_DIR was not set")

  if(NOT DEFINED $ENV{RISCV})
    message(STATUS "RISCV environment variable was not set")
    message(STATUS "Skip adding riscv-tests")
  else()
    set(ISA_TESTS_DIR
        "$ENV{RISCV}/share"
        CACHE PATH "Set riscv-tests build PATH")

    message(STATUS "Set ISA_TESTS_DIR to ${ISA_TESTS_DIR}")
  endif()
endif()

if(DEFINED ISA_TESTS_DIR)
  file(GLOB_RECURSE RUNTIME_TESTS ${ISA_TESTS_DIR}/rv32u[im]-p-*)

  list(FILTER RUNTIME_TESTS EXCLUDE REGEX ".*.dump$")

  foreach(RT_TEST ${RUNTIME_TESTS})
    add_test(NAME ${RT_TEST} COMMAND $<TARGET_FILE:riscv32-sim> ${RT_TEST})
    message(STATUS "Add test ${RT_TEST}")
    set_tests_properties(${RT_TEST} PROPERTIES PASS_REGULAR_EXPRESSION
                                               "Exited with 0x0")
  endforeach()
endif()

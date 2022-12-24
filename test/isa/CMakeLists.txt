add_subdirectory(sim)
add_subdirectory(tvm)

set (ISA_TESTS_DIR "" CACHE PATH "Set riscv-tests build PATH")
file(GLOB_RECURSE RUNTIME_TESTS ${ISA_TESTS_DIR}/rv32u[im]-p-*)

list(FILTER RUNTIME_TESTS EXCLUDE REGEX ".*.dump$")

foreach(RT_TEST ${RUNTIME_TESTS})
  add_test(NAME ${RT_TEST} COMMAND $<TARGET_FILE:riscv32-sim> ${RT_TEST})
  message(STATUS "Add test ${RT_TEST}")
  set_tests_properties(${RT_TEST} PROPERTIES PASS_REGULAR_EXPRESSION
                                             "Exited with 0x0")
endforeach()

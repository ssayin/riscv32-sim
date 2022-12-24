add_subdirectory(sim)
add_subdirectory(tvm)

file(GLOB RUNTIME_TESTS /opt/riscv32/share/rv32u[im]-p-*)

list(FILTER RUNTIME_TESTS EXCLUDE REGEX ".*.dump$")

foreach(RT_TEST ${RUNTIME_TESTS})
  add_test(NAME ${RT_TEST} COMMAND $<TARGET_FILE:riscv32-sim> ${RT_TEST})
  set_tests_properties(${RT_TEST} PROPERTIES PASS_REGULAR_EXPRESSION
                                             "Exited with 0x0")
endforeach()

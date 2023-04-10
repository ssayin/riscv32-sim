# SPDX-FileCopyrightText: 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

function(add_test_executable TARGET SRC)
  add_executable(${TARGET} ${SRC})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_link_libraries(${TARGET} PRIVATE serdar)
endfunction()

function(add_test_with_expected_result TARGET EXPECTED)
  add_riscv32sim_test_target(${TARGET})
  set_tests_properties(${TARGET} PROPERTIES PASS_REGULAR_EXPRESSION
                                            "Exited with ${EXPECTED}")
endfunction()

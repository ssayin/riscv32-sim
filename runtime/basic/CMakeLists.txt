# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

include(riscv)
include(test_config.cmake)

include(riscv)

# set(CMAKE_C_FLAGS "-Os -g -Wall -ffunction-sections")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/crt/serdar.ld")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -fno-exceptions -T ${LINKER_SCRIPT}"
)

add_library(serdar STATIC crt/crt.c)
set_target_properties(serdar PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

set(SOURCES
    src/shift.c
    src/csr_exception.c
    src/fib.c
    src/mul.c
    src/mulhsu.c
    src/divu.c
    src/timer.c)

foreach(SRC ${SOURCES})
  get_filename_component(TARGET ${SRC} NAME_WLE)
  add_executable(${TARGET} ${SRC})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_link_libraries(${TARGET} PRIVATE serdar)
  add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:riscv32-sim>
                                  $<TARGET_FILE:${TARGET}>)
  list(GET ${SRC} 0 EXPECTED)
  set_tests_properties(${TARGET} PROPERTIES PASS_REGULAR_EXPRESSION
                                            "Exited with ${EXPECTED}")
endforeach()

add_test(NAME timer2
         COMMAND $<TARGET_FILE:riscv32-sim> --timer --mtime 32768 --mtimecmp
                 33024 --interval 10 $<TARGET_FILE:timer>)

set_tests_properties(timer2 PROPERTIES FAIL_REGULAR_EXPRESSION
                                       "Exited with 0x0 \(0\)")

# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

include(riscv)
include(cmake/test_config.cmake)
include(cmake/test_helpers.cmake)

set(CMAKE_C_FLAGS "-g -march=rv32im_zicsr -mabi=ilp32")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# set(CMAKE_C_FLAGS "-Os -g -Wall -ffunction-sections")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/crt/serdar.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

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
  list(GET ${SRC} 0 EXPECTED)
  get_filename_component(TARGET ${SRC} NAME_WLE)

  add_test_executable(${TARGET} ${SRC})
  add_test_with_expected_result(${TARGET} ${EXPECTED})
endforeach()

add_test(NAME timer2
         COMMAND $<TARGET_FILE:riscv32-sim> --timer --mtime 32768 --mtimecmp
                 33024 --interval 10 $<TARGET_FILE:timer>)

set_tests_properties(timer2 PROPERTIES FAIL_REGULAR_EXPRESSION
                                       "Exited with 0x0 \(0\)")

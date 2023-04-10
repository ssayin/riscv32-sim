# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

include(riscv)

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/model.ld)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

set(RISCV_ARCH_TEST_DIR
    "${PROJECT_SOURCE_DIR}/external/riscv-arch-test/riscv-test-suite")
set(RISCV_ARCH_TEST_ENV_DIR "${RISCV_ARCH_TEST_DIR}/env")

file(GLOB_RECURSE SOURCES "${RISCV_ARCH_TEST_DIR}/rv32i_m/[IMC]/src/*.S")

foreach(SRC ${SOURCES})
  get_filename_component(TARGET ${SRC} NAME_WLE)
  add_executable(${TARGET} ${SRC} trap_handler.S)
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_include_directories(${TARGET} PRIVATE "${RISCV_ARCH_TEST_ENV_DIR}")
  target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  message(STATUS "Add test target ${TARGET}")
  add_riscv32sim_test_target(${TARGET})
endforeach()

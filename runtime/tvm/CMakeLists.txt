# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

include(riscv)

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(RISCV_TEST_ENV_DIR "${PROJECT_SOURCE_DIR}/external/riscv-test-env")
set(LINKER_SCRIPT "${RISCV_TEST_ENV_DIR}/p/link.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

set(SOURCES test1.S test2.S)

foreach(SRC ${SOURCES})
  get_filename_component(TARGET ${SRC} NAME_WLE)
  add_executable(${TARGET} ${SRC})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_include_directories(${TARGET} PRIVATE "${RISCV_TEST_ENV_DIR}")
  add_riscv32sim_test_target(${TARGET})
endforeach()

include(riscv)
include(riscv_test_env)

set(SOURCES test1.S)

set(SIM_BUILD_DIR "${PROJECT_BINARY_DIR}/riscv32-sim")
set(SIM_PROGRAM "${SIM_BUILD_DIR}/riscv32-sim")

foreach(SRC ${SOURCES})
  get_filename_component(TARGET ${SRC} NAME_WLE)
  add_executable(${TARGET} ${SRC})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_include_directories(${TARGET} PRIVATE "${RISCV_TEST_ENV_DIR}")
  add_test(NAME ${TARGET} COMMAND ${SIM_PROGRAM} $<TARGET_FILE:${TARGET}>)
endforeach()

include(riscv)

# set(CMAKE_C_FLAGS "-Os -g -Wall -ffunction-sections")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nodefaultlibs -fno-exceptions -T ${LINKER_SCRIPT}"
)

set(SIM_BUILD_DIR "${PROJECT_BINARY_DIR}/riscv32-sim")
set(SIM_PROGRAM "${SIM_BUILD_DIR}/riscv32-sim")

add_library(serdar STATIC crt/crt.c)
set_target_properties(serdar PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

set(SOURCES example/shift.c example/csr_exception.c example/fib.c)

list(APPEND example/shift.c 24)
list(APPEND example/csr_exception.c 0)
list(APPEND example/fib.c 13)

foreach(SRC ${SOURCES})
  get_filename_component(TARGET ${SRC} NAME_WLE)
  add_executable(${TARGET} ${SRC})
  set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")
  target_link_libraries(${TARGET} PRIVATE serdar)
  add_test(NAME ${TARGET} COMMAND ${SIM_PROGRAM} $<TARGET_FILE:${TARGET}>)
  list(GET ${SRC} 0 EXPECTED)
  set_tests_properties(${TARGET} PROPERTIES PASS_REGULAR_EXPRESSION "Exited with ${EXPECTED}")
endforeach()

project(riscv32i-sim-build)
include(../cmake/riscv32i-sim.cmake)

set(RISCV32I_FLAGS -nostdlib -nodefaultlibs)

add_executable(test test.c)

target_compile_options(test PRIVATE ${RISCV32I_FLAGS})
target_link_options(test PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})

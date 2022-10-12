project(riscv32i-sim-build)
include(../cmake/riscv32i-sim.cmake)

if(FOUND_RISCV_GCC)
  message(STATUS "unknown-elf-riscv32-gcc found, building spike")
  set(RISCV32I_FLAGS -nostdlib -nodefaultlibs)

  add_executable(test test.c)

  target_compile_options(test PRIVATE ${RISCV32I_FLAGS})
  target_link_options(test PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})
else()
  message(STATUS "unknown-elf-riscv32-gcc not found, skipping building spike")
endif()

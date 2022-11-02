project(riscv32i-sim-build)
include(../cmake/riscv32i-sim.cmake)

if(FOUND_RISCV_GCC)
  message(STATUS "unknown-elf-riscv32-gcc found, building spike")
  set(RISCV32I_FLAGS -nostdlib -nodefaultlibs)


  add_library(serdar STATIC start.c)

  add_executable(test test.c)
  add_executable(test_csr test_csr.c)

  target_link_libraries(test PRIVATE serdar)
  target_link_libraries(test_csr PRIVATE serdar)

  target_compile_options(test PRIVATE ${RISCV32I_FLAGS})
  target_link_options(test PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})

  target_compile_options(test_csr PRIVATE ${RISCV32I_FLAGS})
  target_link_options(test_csr PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})

else()
  message(STATUS "unknown-elf-riscv32-gcc not found, skipping building spike")
endif()

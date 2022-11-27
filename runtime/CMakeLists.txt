project(riscv32-sim-build)
include(../cmake/riscv32-sim.cmake)

if (FOUND_RISCV_GCC)
    message(STATUS "unknown-elf-riscv32-gcc found, building runtime")
    set(RISCV32I_FLAGS -nostdlib -nodefaultlibs)


    add_library(serdar STATIC crt/crt.c)

    add_executable(test test/test.c)
    add_executable(test_csr test/test_csr.c)

    target_link_libraries(test PRIVATE serdar)
    target_link_libraries(test_csr PRIVATE serdar)

    target_compile_options(test PRIVATE ${RISCV32I_FLAGS})
    target_link_options(test PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})

    target_compile_options(test_csr PRIVATE ${RISCV32I_FLAGS})
    target_link_options(test_csr PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/serdar.ld ${RISCV32I_FLAGS})

else ()
    message(STATUS "unknown-elf-riscv32-gcc not found, skipping building runtime")
endif ()

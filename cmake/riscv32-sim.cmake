set(CMAKE_SYSTEM_NAME riscv32-sim)

set(CMAKE_FIND_ROOT_PATH /opt/riscv32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

find_program(FOUND_RISCV_GCC "riscv32-unknown-elf-gcc")
find_program(FOUND_RISCV_CXX "riscv32-unknown-elf-g++")

if (FOUND_RISCV_GCC)
    set(CMAKE_C_COMPILER riscv32-unknown-elf-gcc)
endif ()


if (FOUND_RISCV_CXX)
    set(CMAKE_CXX_COMPILER riscv32-unknown-elf-g++)
endif ()

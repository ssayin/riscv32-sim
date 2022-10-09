add_executable(riscv32i-sim main.cpp src/computer.cpp src/rv32_decode.cpp)
target_link_libraries(riscv32i-sim elfio)
target_include_directories(riscv32i-sim PRIVATE include)

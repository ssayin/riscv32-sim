add_executable(riscv32i-sim main.cpp src/computer.cpp)
target_link_libraries(riscv32i-sim elfio)
target_include_directories(riscv32i-sim PRIVATE include)

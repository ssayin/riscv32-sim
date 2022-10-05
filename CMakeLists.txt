cmake_minimum_required(VERSION 3.12)

project(
  riscv32i-sim
  LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(external)

add_executable(riscv32i-sim src/main.cpp)
set_property(TARGET riscv32i-sim PROPERTY CXX_STANDARD 20)
set_property(TARGET riscv32i-sim PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(riscv32i-sim elfio)

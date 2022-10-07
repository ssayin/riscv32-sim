cmake_minimum_required(VERSION 3.12)

project(
  riscv32i-sim
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(external)
add_subdirectory(riscv32i-sim)

cmake_minimum_required(VERSION 3.12)

project(riscv32-sim LANGUAGES CXX C ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_subdirectory(external)
add_subdirectory(riscv32-sim)

option(BUILD_TESTS "" OFF)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()

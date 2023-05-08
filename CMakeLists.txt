# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.23.2)
configure_file(config.hpp.in config.hpp)

# probably not the very best practice
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(${CMAKE_VERSION} VERSION_GREATER 3.24)
  cmake_policy(SET CMP0135 NEW)
endif()

project(riscv32-sim LANGUAGES CXX C ASM)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)

  option(ENABLE_TCP "" OFF)
  option(BUILD_RUNTIME "" OFF)
  option(COVERAGE "" OFF)

  option(ENABLE_UBENCHMARKS "" OFF)

  option(TEST_EXPORT_JSON_HART_STATE "" OFF)
  option(TEST_EXPORT_TRACE_DISASSEMBLY "" OFF)

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

  set(RISCV32_SIM_TRACE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/log)
  file(MAKE_DIRECTORY ${RISCV32_SIM_TRACE_OUTPUT_DIRECTORY})

  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  if(COVERAGE)
    if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
      message(
        WARNING
          "Coverage analysis in ${CMAKE_BUILD_TYPE} builds may not produce accurate results."
          " Specify -DCMAKE_BUILD_TYPE=Debug to configure the build type as Debug."
      )
    endif()
  endif()

  add_subdirectory(external)
  add_subdirectory(riscv32-sim)

  if(BUILD_TESTING)
    if(BUILD_RUNTIME)
      add_subdirectory(runtime)
    endif()
  endif()
endif()

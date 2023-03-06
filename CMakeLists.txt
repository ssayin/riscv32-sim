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

  if(BUILD_TESTING)
    add_subdirectory(riscv-tests)
    if(BUILD_RUNTIME)
      add_subdirectory(runtime)
    endif()
  endif()
  add_subdirectory(riscv32-sim)
endif()

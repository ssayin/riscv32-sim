cmake_minimum_required(VERSION 3.23.2)

if(${CMAKE_VERSION} VERSION_GREATER 3.24)
  cmake_policy(SET CMP0135 NEW)
endif()

project(riscv32-sim LANGUAGES CXX C ASM)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(BUILD_TESTS "" OFF)
  option(COVERAGE "" OFF)

  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  if(BUILD_TESTS)
    include(CTest)
  endif()
endif()

add_subdirectory(external)
add_subdirectory(riscv32-sim)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
  add_subdirectory(test)
endif()

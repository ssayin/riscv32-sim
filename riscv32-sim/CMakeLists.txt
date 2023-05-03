# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

if(BUILD_TESTING)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker
  # settings
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  include(GoogleTest)

  macro(add_unit_test TEST)
    add_executable(${TEST} ${ARGN})
    target_link_libraries(${TEST} GTest::gtest_main riscv32-sim-common)

    target_include_directories(
      ${TEST} PRIVATE ${PROJECT_SOURCE_DIR}/riscv32-sim/include)
    gtest_discover_tests(${TEST})
  endmacro()
endif()

add_subdirectory(lib)

set(SRC_FILES
    src/main.cpp
    src/iss/model.cpp
    src/memory/sparse_memory.cpp
    src/loader.cpp
    include/zicsr/csr.hpp
    include/iss/model.hpp
    include/loader.hpp
    include/zicsr/misa.hpp
    include/memory/sparse_memory.hpp
    include/trap_cause.hpp
    src/reg_file.cpp
    include/reg_file.hpp
    include/zicsr/csr_file.hpp
    src/mti_source.cpp
    include/mti_source.hpp
    include/util/format_helpers.hpp)

if(ENABLE_TCP)
  add_library(rv32_sim_ipc OBJECT src/ipc.cpp)
  target_include_directories(rv32_sim_ipc PUBLIC include)
  target_link_libraries(rv32_sim_ipc PUBLIC boost_asio)
endif()

set(TCP_LIB $<$<BOOL:${ENABLE_TCP}>:rv32_sim_ipc>)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE elfio::elfio
          CLI11
          riscv_disas
          nlohmann_json::nlohmann_json
          spdlog::spdlog
          $<$<BOOL:${MINGW}>:ws2_32>
          ${TCP_LIB}
          riscv32-sim-common
          riscv32-decoder)

target_include_directories(${PROJECT_NAME} PRIVATE include)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE "-Wall;-Wpedantic;-Werror")
  target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:Release>:-O2>")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

endif()

if(COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME}
                           PRIVATE "-fprofile-arcs;-ftest-coverage")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-lgcov")
  endif()
endif()

if(BUILD_TESTING)
  add_subdirectory(unittest)
endif()

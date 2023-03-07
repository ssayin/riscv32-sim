# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

if(ENABLE_TCP)
  add_library(rv32-irq-server OBJECT src/ipc/irq_server.cpp)
  target_include_directories(rv32-irq-server PUBLIC include)

  add_executable(client src/ipc/client.cpp)
  target_link_libraries(client rv32-irq-server)

  add_executable(minimal src/ipc/minimal_test.cpp)
  target_link_libraries(minimal rv32-irq-server)

  add_test(NAME minimal COMMAND minimal)
endif()

add_subdirectory(riscv32-sim-common)
add_subdirectory(riscv32-decoder)

set(SRC_FILES
    src/main.cpp
    src/iss_model.cpp
    src/memory/sparse_memory.cpp
    src/loader.cpp
    include/zicsr/csr.hpp
    include/iss_model.hpp
    include/loader.hpp
    include/zicsr/misa.hpp
    include/memory/sparse_memory.hpp
    include/trap_cause.hpp
    src/reg_file.cpp
    include/reg_file.hpp
    include/zicsr/csr_file.hpp
    src/mti_source.cpp
    include/mti_source.hpp)

set(TCP_LIB $<$<BOOL:${ENABLE_TCP}>:rv32-irq-server>)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE elfio::elfio
          fmt::fmt
          CLI11
          riscv_disas
          nlohmann_json::nlohmann_json
          ${TCP_LIB}
          riscv32-sim-common
          riscv32-decoder)

target_include_directories(${PROJECT_NAME} PRIVATE include)

add_executable(json_test_op src/json_test_op.cpp)
target_link_libraries(json_test_op PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(json_test_op PRIVATE include)

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

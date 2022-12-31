set(SRC_FILES
    src/main.cpp
    src/iss_model.cpp
    src/decoder/decoder.cpp
    src/memory/sparse_memory.cpp
    src/loader.cpp
    include/zicsr/csr.hpp
    include/decoder/decoder.hpp
    include/iss_model.hpp
    include/loader.hpp
    include/zicsr/misa.hpp
    include/instr/rv32_isn.hpp
    include/memory/sparse_memory.hpp
    include/common/types.hpp
    include/zicsr/trap_cause.hpp
    include/common/offset.hpp
    src/reg_file.cpp
    include/reg_file.hpp
    include/common/consts.hpp
    include/zicsr/csr_file.hpp
    src/zicsr/csr_file.cpp
    include/zicsr/sync_exception.hpp
    include/zicsr/privilege_level.hpp
    src/zicsr/csr_file.cpp)

add_executable(riscv32-sim ${SRC_FILES})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME
   AND BUILD_TESTS
   AND COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Werror")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}") # change later
endif()

target_link_libraries(riscv32-sim PRIVATE elfio fmt::fmt)
target_include_directories(riscv32-sim PRIVATE include)

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
    include/zicsr/privilege.hpp
    src/zicsr/csr_file.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE elfio fmt::fmt)
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

if(BUILD_TESTS)
  add_subdirectory(unittest)
endif()

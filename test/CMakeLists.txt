cmake_minimum_required(VERSION 3.12)

include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

macro(add_unit_test TEST)
    add_executable(
            ${TEST}
            ${ARGN}
    )
    target_link_libraries(
            ${TEST}
            GTest::gtest_main
            fmt::fmt
    )
    target_include_directories(${TEST} PRIVATE ../riscv32-sim/include)
    gtest_discover_tests(${TEST})
endmacro()

add_unit_test(sparse_memory_test sparse_memory_test.cc ${PROJECT_SOURCE_DIR}/riscv32-sim/src/sparse_memory.cpp)

set(
  SRC_FILES
  src/main.cpp
  src/iss_model.cpp
  src/decoder.cpp
  src/sparse_memory.cpp
  src/loader.cpp
)

add_executable(riscv32i-sim ${SRC_FILES})

target_link_libraries(riscv32i-sim elfio fmt::fmt)
target_include_directories(riscv32i-sim PRIVATE include)

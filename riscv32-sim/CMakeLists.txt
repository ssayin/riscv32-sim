set(
        SRC_FILES
        src/main.cpp
        src/iss_model.cpp
        src/decoder.cpp
        src/sparse_memory.cpp
        src/loader.cpp
        include/csr.hpp
        include/decoder.hpp
        include/iss_model.hpp
        include/loader.hpp
        include/masks.hpp
        include/misa.hpp
        include/rv32_isn.hpp
        include/sparse_memory.hpp
        include/types.hpp
        include/trap_cause.hpp include/config.hpp include/offset.hpp src/reg_file.cpp include/reg_file.hpp)

add_executable(riscv32-sim ${SRC_FILES})

target_link_libraries(riscv32-sim elfio fmt::fmt)
target_include_directories(riscv32-sim PRIVATE include)

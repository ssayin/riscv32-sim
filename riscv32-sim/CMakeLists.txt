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

target_link_libraries(riscv32-sim elfio fmt::fmt)
target_include_directories(riscv32-sim PRIVATE include)

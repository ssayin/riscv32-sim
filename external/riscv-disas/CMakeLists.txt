add_library(RISCV_DISAS STATIC)

target_sources(RISCV_DISAS PRIVATE riscv-disas.c)
target_include_directories(RISCV_DISAS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

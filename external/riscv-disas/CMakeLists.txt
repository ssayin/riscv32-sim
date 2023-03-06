# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

add_library(riscv_disas STATIC)

target_sources(riscv_disas PRIVATE riscv-disas.c)
target_include_directories(riscv_disas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

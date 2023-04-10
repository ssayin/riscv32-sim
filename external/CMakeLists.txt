# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

add_subdirectory(ELFIO)
add_subdirectory(fmt)
add_subdirectory(riscv-disas)
add_subdirectory(CLI11)
add_subdirectory(json)
add_subdirectory(spdlog)

add_library(boost_asio INTERFACE)
add_library(Boost::asio ALIAS boost_asio)

target_include_directories(boost_asio INTERFACE asio/include)

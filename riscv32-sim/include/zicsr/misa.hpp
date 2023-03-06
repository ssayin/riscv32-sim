// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef ZICSR_MISA_HPP
#define ZICSR_MISA_HPP

#include <cstdint>
#include <numeric>
#include <string_view>

constexpr uint32_t operator"" _MISA(const char *isa, std::size_t) {
  std::string_view str{isa};
  auto             sel = [](uint32_t acc, char c) -> uint32_t {
    return acc | ((c >= 'A' && c <= 'Z') ? 1 << (25 - ('Z' - c)) : 0);
  };
  return std::accumulate(str.begin(), str.end(), 0x0, sel);
}

constexpr static uint32_t rv32 = (1 << 30);

constinit static uint32_t misa_value = "IMS"_MISA | rv32;

#endif // ZICSR_MISA_HPP

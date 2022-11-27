#ifndef RISCV32_SIM_MISA_HPP
#define RISCV32_SIM_MISA_HPP

#include <cstdint>
#include <numeric>
#include <string_view>

consteval uint32_t operator"" _MISA(const char *isa, size_t) {
  std::string_view str{isa};
  auto             sel = [](uint32_t acc, char c) -> uint32_t {
    return acc | ((c >= 'A' && c <= 'Z') ? 1 << (25 - ('Z' - c)) : 0);
  };
  return std::accumulate(str.begin(), str.end(), 0x0, sel);
}

constexpr uint32_t RV32 = (1 << 30);

#endif // RISCV32_SIM_MISA_HPP

#ifndef RISCV32_SIM_OFFSET_HPP
#define RISCV32_SIM_OFFSET_HPP

#include "types.hpp"

consteval auto fillbits(Unsigned auto bitcount) {
  if (bitcount == 0u)
    static_assert("fillbits: number of bits cannot be 0");
  auto sum = 1u;
  while (--bitcount) {
    sum = (sum << 1u) | 1u;
  }
  return sum;
}

template <Unsigned auto l, Unsigned auto h>
using diff = std::integral_constant<decltype(h - l + 1u), h - l + 1u>;

template <Unsigned auto l, Unsigned auto h>
constexpr decltype(auto) offset(Unsigned auto inst) {
  return (inst >> l) & fillbits(diff<l, h>::value);
}

#define FUNCT7(word) offset<25u, 31u>(word)
#define RS2(word) offset<20u, 24u>(word)
#define RS1(word) offset<15u, 19u>(word)
#define FUNCT3(word) offset<12u, 14u>(word)
#define RD(word) offset<7u, 11u>(word)
#define OPCODE(word) offset<0u, 6u>(word)


#endif // RISCV32_SIM_OFFSET_HPP

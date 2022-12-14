#ifndef RISCV32_SIM_OFFSET_HPP
#define RISCV32_SIM_OFFSET_HPP

#include "common/types.hpp"

consteval auto fillbits(Unsigned auto bitcount) {
  auto sum = 1U;
  while (--bitcount) {
    sum = (sum << 1U) | 1U;
  }
  return sum;
}

template <Unsigned auto low, Unsigned auto high>
using diff = std::integral_constant<decltype(high - low + 1U), high - low + 1U>;

template <Unsigned auto low, Unsigned auto high>
constexpr decltype(auto) offset(Unsigned auto inst) {
  return (inst >> low) & fillbits(diff<low, high>::value);
}

constexpr auto to_int(Enum auto val) {
  return static_cast<std::underlying_type_t<decltype(val)>>(val);
}

#define FUNCT_7(word) offset<25U, 31U>(word)
#define RS_2(word)    offset<20U, 24U>(word)
#define RS_1(word)    offset<15U, 19U>(word)
#define FUNCT_3(word) offset<12U, 14U>(word)
#define RD(word)      offset<7U, 11U>(word)
#define OPCODE(word)  offset<0U, 6U>(word)

#endif // RISCV32_SIM_OFFSET_HPP

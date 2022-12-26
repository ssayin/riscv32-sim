#ifndef COMMON_OFFSET_HPP
#define COMMON_OFFSET_HPP

#include "common/types.hpp"

constexpr auto fillbits(UnsignedIntegral auto bitcount) {
  auto sum = 1U;
  while (--bitcount) {
    sum = (sum << 1U) | 1U;
  }
  return sum;
}

template <UnsignedIntegral auto low, UnsignedIntegral auto high>
using diff = std::integral_constant<decltype(high - low + 1U), high - low + 1U>;

template <UnsignedIntegral auto low, UnsignedIntegral auto high>
constexpr decltype(auto) offset(UnsignedIntegral auto inst) {
  return (inst >> low) & fillbits(diff<low, high>::value);
}

constexpr auto to_int(Enum auto val) {
  return static_cast<std::underlying_type_t<decltype(val)>>(val);
}

namespace off {
constexpr uint8_t funct7(uint32_t w) { return offset<25U, 31U>(w); }
constexpr uint8_t funct3(uint32_t w) { return offset<12U, 14U>(w); }
constexpr uint8_t rs2(uint32_t w) { return offset<20U, 24U>(w); }
constexpr uint8_t rs1(uint32_t w) { return offset<15U, 19U>(w); }
constexpr uint8_t opc(uint32_t w) { return offset<0U, 6u>(w); }
constexpr uint8_t rd(uint32_t w) { return offset<7U, 11U>(w); }
}; // namespace off

#endif // COMMON_OFFSET_HPP

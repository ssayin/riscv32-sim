#ifndef COMMON_OFFSET_HPP
#define COMMON_OFFSET_HPP

#include "common/types.hpp"

constexpr auto fillbits(UnsignedIntegral auto bitcount) {
  return (1U << bitcount) - 1U;
}

constexpr auto offset(UnsignedIntegral auto inst, UnsignedIntegral auto low,
                      UnsignedIntegral auto high) {
  return (inst >> low) & fillbits(high - low + 1U);
}

constexpr auto to_int(Enum auto val) {
  return static_cast<std::underlying_type_t<decltype(val)>>(val);
}

namespace off {
constexpr uint8_t funct7(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 25U, 31U));
}
constexpr uint8_t funct3(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 12U, 14U));
}
constexpr uint8_t rs2(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 20U, 24U));
}
constexpr uint8_t rs1(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 15U, 19U));
}
constexpr uint8_t opc(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 0U, 6U));
}
constexpr uint8_t rd(uint32_t w) {
  return static_cast<uint8_t>(offset(w, 7U, 11U));
}
}; // namespace off

#endif // COMMON_OFFSET_HPP

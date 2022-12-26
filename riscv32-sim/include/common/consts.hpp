#ifndef COMMON_CONSTS_HPP
#define COMMON_CONSTS_HPP

#include <cstdint>

struct masks {

  static constexpr uint32_t sign_bit   = 0x80000000U;
  static constexpr uint32_t msb_zero   = 0x7FFFFFFFU;
  static constexpr uint32_t type_u_imm = 0xFFFFF000U;

  struct tvec {
    static constexpr uint32_t type      = 0x00000002U;
    static constexpr uint32_t base_addr = 0xFFFFFFFCU;
  };
};

#endif // COMMON_CONSTS_HPP

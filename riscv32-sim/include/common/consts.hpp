#ifndef RISCV32_SIM_CONSTS_HPP
#define RISCV32_SIM_CONSTS_HPP

#include <cstdint>

struct masks {
  struct tvec {
    static constexpr uint32_t type      = 0x00000002U;
    static constexpr uint32_t base_addr = 0xFFFFFFFCU;
  };

  static constexpr uint32_t sign_bit   = 0x80000000U;
  static constexpr uint32_t msb_zero   = 0x7FFFFFFFU;
  static constexpr uint32_t type_u_imm = 0xFFFFF000U;
};

#endif // RISCV32_SIM_CONSTS_HPP

#ifndef RISCV32_SIM_CONSTS_HPP
#define RISCV32_SIM_CONSTS_HPP

#include <cstdint>

struct consts {
  static constexpr uint32_t sign_bit_mask       = 0x80000000U;
  static constexpr uint32_t msb_zero_mask       = 0x7FFFFFFFU;
  static constexpr uint32_t tvec_type_mask      = 0x00000002U;
  static constexpr uint32_t tvec_base_addr_mask = 0xFFFFFFFCU;
  static constexpr uint32_t type_u_imm_mask     = 0xFFFFF000U;

  struct status {
    static constexpr uint8_t mie  = 3U;
    static constexpr uint8_t mpie = 7U;
    static constexpr uint8_t mpp  = 11U;
    static constexpr uint8_t mprv = 17U;
    static constexpr uint8_t sie  = 1U;
    static constexpr uint8_t spie = 5U;
    static constexpr uint8_t spp  = 8U;
  };
};

#endif // RISCV32_SIM_CONSTS_HPP

#ifndef RISCV32_SIM_CONSTS_HPP
#define RISCV32_SIM_CONSTS_HPP

#include <cstdint>

struct consts {
  static constexpr uint32_t sign_bit_mask       = 0x80000000;
  static constexpr uint32_t msb_zero_mask       = 0x7FFFFFFF;
  static constexpr uint32_t tvec_type_mask      = 0x00000002;
  static constexpr uint32_t tvec_base_addr_mask = 0xFFFFFFFC;

  static constexpr uint8_t status_mie  = 3;
  static constexpr uint8_t status_mpie = 7;
  static constexpr uint8_t status_mpp  = 11;
  static constexpr uint8_t status_mprv = 17;
  static constexpr uint8_t status_sie  = 1;
  static constexpr uint8_t status_spie = 5;
  static constexpr uint8_t status_spp  = 8;

  static constexpr uint32_t type_u_imm_mask = 0xFFFFF000;
};

#endif // RISCV32_SIM_CONSTS_HPP

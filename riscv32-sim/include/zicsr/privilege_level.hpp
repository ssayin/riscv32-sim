#ifndef RISCV32_SIM_PRIVILEGE_LEVEL_HPP
#define RISCV32_SIM_PRIVILEGE_LEVEL_HPP

enum class privilege_level {
  user       = 0b00,
  supervisor = 0b01,
  hypervisor = 0b10,
  machine    = 0b11
};

#endif // RISCV32_SIM_PRIVILEGE_LEVEL_HPP

#ifndef RISCV32_SIM_CONFIG_HPP
#define RISCV32_SIM_CONFIG_HPP

#include "misa.hpp"

constinit static uint32_t misa_value = "IMS"_MISA | RV32;

#endif // RISCV32_SIM_CONFIG_HPP
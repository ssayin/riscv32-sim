#ifndef RISCV32_SIM_CSR_FILE_HPP
#define RISCV32_SIM_CSR_FILE_HPP

#include "privilege_level.hpp"
#include <array>
#include <cstdint>

class csr_file {
  std::array<uint32_t, 4096> csrs{};
  const privilege_level     &mode;

public:
  explicit csr_file(const privilege_level &mode) : mode(mode) {}
  uint32_t read(uint32_t addr);
  void     write(uint32_t addr, uint32_t v);
};

#endif // RISCV32_SIM_CSR_FILE_HPP
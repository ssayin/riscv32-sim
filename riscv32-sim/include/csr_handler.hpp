#ifndef RISCV32_SIM_CSR_HANDLER_HPP
#define RISCV32_SIM_CSR_HANDLER_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "csr.hpp"
#include "misa.hpp"

class csr_handler {
  static bool                is_unimplemented_csr(uint32_t addr);
  std::array<uint32_t, 4096> csrs{};

public:
  csr_handler();
  uint32_t read(uint32_t addr);
  uint32_t read(csr addr);
  void     write(uint32_t addr, uint32_t v);
  void     write(csr addr, uint32_t v);

  enum priv {
    user [[maybe_unused]] = 0,
    supervisor [[maybe_unused]],
    reserved [[maybe_unused]],
    machine
  };

  priv mode = machine;
};

#endif // RISCV32_SIM_CSR_HANDLER_HPP

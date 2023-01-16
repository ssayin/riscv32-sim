#ifndef ZICSR_CSR_FILE_HPP
#define ZICSR_CSR_FILE_HPP

#include "common/types.hpp"
#include "csr.hpp"
#include "privilege.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

struct status {
  static constexpr uint8_t sie  = 1U;
  static constexpr uint8_t mie  = 3U;
  static constexpr uint8_t spie = 5U;
  static constexpr uint8_t spp  = 8U;
  static constexpr uint8_t mpie = 7U;
  static constexpr uint8_t mpp  = 11U;
  static constexpr uint8_t mprv = 17U;
};

class csr_file {
  static constexpr size_t         csr_count = 4096U;
  std::array<uint32_t, csr_count> csrs{};

public:
  uint32_t read(uint32_t addr) { return csrs.at(addr); }
  void     write(uint32_t addr, uint32_t v) { csrs.at(addr) = v; }
};

#endif // ZICSR_CSR_FILE_HPP

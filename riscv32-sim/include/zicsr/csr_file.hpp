#ifndef ZICSR_CSR_FILE_HPP
#define ZICSR_CSR_FILE_HPP

#include "privilege_level.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

struct status {
  static constexpr uint8_t mie  = 3U;
  static constexpr uint8_t mpie = 7U;
  static constexpr uint8_t mpp  = 11U;
  static constexpr uint8_t mprv = 17U;
  static constexpr uint8_t sie  = 1U;
  static constexpr uint8_t spie = 5U;
  static constexpr uint8_t spp  = 8U;
};

class csr_file {
  static constexpr size_t         csr_count = 4096U;
  std::array<uint32_t, csr_count> csrs{};
  const privilege_level          &mode;

public:
  explicit csr_file(const privilege_level &mode) : mode{mode} {}
  uint32_t read(uint32_t addr);
  void     write(uint32_t addr, uint32_t v);
};

#endif // ZICSR_CSR_FILE_HPP

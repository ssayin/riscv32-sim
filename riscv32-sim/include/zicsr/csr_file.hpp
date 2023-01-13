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
  const privilege                &mode;

public:
  explicit csr_file(const privilege &mode) : mode{mode} {}
  uint32_t read(uint32_t addr);
  void     write(uint32_t addr, uint32_t v);

  uint16_t priv_base() const {
    return static_cast<uint16_t>(to_int(mode) << 8);
  }

  uint16_t priv_csr(enum csr c) const { return priv_base() | to_int(c); }
};

#endif // ZICSR_CSR_FILE_HPP

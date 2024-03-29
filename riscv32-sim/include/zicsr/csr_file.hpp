// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef ZICSR_CSR_FILE_HPP
#define ZICSR_CSR_FILE_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <limits>

#include "common/common.hpp"
#include "zicsr/csr.hpp"

struct status {
  static constexpr uint8_t sie  = 1U;
  static constexpr uint8_t mie  = 3U;
  static constexpr uint8_t spie = 5U;
  static constexpr uint8_t spp  = 8U;
  static constexpr uint8_t mpie = 7U;
  static constexpr uint8_t mpp  = 11U;
  static constexpr uint8_t mprv = 17U;
};

enum class privilege {
  user       = 0b00,
  supervisor = 0b01,
  hypervisor = 0b10,
  machine    = 0b11
};

class privileged_mode {
  privilege m{};
  uint16_t  base{};

public:
  privileged_mode(privilege m) { set_mode(m); }
  [[nodiscard]] privilege mode() const { return m; }
  static uint8_t          priv(uint32_t addr) { return offset(addr, 8U, 9U); };

  [[nodiscard]] bool can_write(uint32_t addr) const {
    return !readonly(addr) && (priv(addr) <= to_int(m));
  }

  [[nodiscard]] bool can_read(uint32_t addr) const {
    return priv(addr) <= to_int(m);
  }

  static bool readonly(uint32_t addr) { return offset(addr, 10U, 11U) == 0b11; }

  void set_mode(privilege mode) {
    m    = mode;
    base = static_cast<uint16_t>(static_cast<uint16_t>(m) << 8);
  }

  [[nodiscard]] uint16_t priv_csr(enum csr c) const { return base | to_int(c); }
};

class csr_file {
  static constexpr size_t         csr_count = 4096U;
  std::array<uint32_t, csr_count> csrs{};

public:
  uint32_t read(uint32_t addr) { return csrs.at(addr); }
  void     write(uint32_t addr, uint32_t v) { csrs.at(addr) = v; }
  void     write(uint16_t addr, uint32_t v, csr_change &out) {
    out.index = addr;
    out.next  = v;
    out.prev  = csrs.at(addr);
    write(addr, v);
  }

  void write(uint16_t addr, uint32_t v, std::vector<csr_change> &vec) {
    csr_change tmp;
    write(addr, v, tmp);
    vec.emplace_back(tmp);
  }
};

#endif // ZICSR_CSR_FILE_HPP

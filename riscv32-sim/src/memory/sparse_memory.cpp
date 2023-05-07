// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include <cstring>

#include "common/common.hpp"
#include "memory/sparse_memory.hpp"

namespace mem {
// address_router constructor implementation
address_router::address_router(sparse_memory &mem, uint32_t mtime_addr,
                               uint32_t mtimecmp_addr)
    : sparse_memory_accessor<address_router>{mem}, mtime_addr{mtime_addr},
      mtimecmp_addr{mtimecmp_addr} {}

// address_router read8 and write8 implementation
uint8_t address_router::read8(uint32_t off) const {
  if (off >= mtime_addr && off < mtime_addr + 8) {
    return mtime.at(off - mtime_addr).load();
  }
  if (off >= mtimecmp_addr && off < mtimecmp_addr + 8) {
    return mtimecmp.at(off - mtimecmp_addr).load();
  }
  return sparse_memory_accessor::read8(off);
}

void address_router::write8(uint32_t off, uint8_t b) {
  if (off >= mtime_addr && off < mtime_addr + 8) {
    mtime.at(off - mtime_addr).store(b);
  } else if (off >= mtimecmp_addr && off < mtimecmp_addr + 8) {
    mtimecmp.at(off - mtimecmp_addr).store(b);
  } else {
    sparse_memory_accessor::write8(off, b);
  }
}
} // namespace mem

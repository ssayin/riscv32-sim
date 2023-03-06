// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#include "memory/sparse_memory.hpp"
#include "common/offset.hpp"

#include <cstring>

static void *write_block(uint8_t *page_offset, void *ptr,
                         uint32_t size_in_bytes) {
  std::memcpy(page_offset, ptr, size_in_bytes);
  return (uint8_t *)ptr + size_in_bytes;
}

uint32_t sparse_memory_accessor::ensure_page_exists(uint32_t addr) {
  uint32_t key = addr & mask;
  if (!page.contains(key))
    page.emplace(key, std::make_unique<uint8_t[]>(page_size));
  return key;
}

void sparse_memory_accessor::write(uint32_t virt_addr, void *ptr,
                                   int64_t size_in_bytes) {
  while (size_in_bytes > 0) {
    uint32_t key      = ensure_page_exists(virt_addr);
    uint32_t off      = offset(virt_addr, 0U, 11U);
    uint32_t page_end = page_size - off;
    ptr               = write_block(&page[key].get()[off], ptr, page_end);
    size_in_bytes -= page_end;
    virt_addr += page_end;
  }
}

uint8_t sparse_memory_accessor::read8(uint32_t off) const {
  if (!page.contains(off & mask)) {
    return 0U;
  }
  return page.at(off & mask).get()[offset(off, 0U, 11U)];
}

uint16_t sparse_memory_accessor::read16(uint32_t off) const {
  return read8(off) | (read8(off + 1) << 8);
}

uint32_t sparse_memory_accessor::read32(uint32_t off) const {
  return read16(off) | (read16(off + 2) << 16);
}

void sparse_memory_accessor::write8(uint32_t off, uint8_t b) {
  uint32_t key                          = ensure_page_exists(off);
  page[key].get()[offset(off, 0U, 11U)] = b;
}

void sparse_memory_accessor::write16(uint32_t off, uint16_t h) {
  write8(off, offset(h, 0U, 7U));
  write8(off + 1, offset(h, 8U, 15U));
}

void sparse_memory_accessor::write32(uint32_t off, uint32_t w) {
  write16(off, offset(w, 0U, 15U));
  write16(off + 2, offset(w, 16U, 31U));
}

uint64_t sparse_memory_accessor::read64(uint32_t off) const {
  return (read32(off)) | (static_cast<uint64_t>(read32(off + 4)) << 32);
}

void sparse_memory_accessor::write64(uint32_t off, uint64_t dw) {
  write32(off, (dw << 32) >> 32);
  write32(off + 4, dw >> 32);
}

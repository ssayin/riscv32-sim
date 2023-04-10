// SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
//
// SPDX-License-Identifier: MIT

#ifndef MEMORY_SPARSE_MEMORY_HPP
#define MEMORY_SPARSE_MEMORY_HPP

#include "common/offset.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>

namespace mem {
using sparse_memory = std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>>;

template <typename Derived> class sparse_memory_accessor_base {
protected:
  sparse_memory &page;

public:
  constexpr static uint32_t page_size = 4096U;
  constexpr static uint32_t mask      = 0xFFFFF000U;

  sparse_memory_accessor_base(const sparse_memory_accessor_base &) = default;
  sparse_memory_accessor_base(sparse_memory_accessor_base &&)      = delete;
  sparse_memory_accessor_base &
  operator=(const sparse_memory_accessor_base &) = delete;
  sparse_memory_accessor_base &
  operator=(sparse_memory_accessor_base &&) = delete;
  explicit sparse_memory_accessor_base(sparse_memory &page) : page{page} {}
  virtual ~sparse_memory_accessor_base() = default;

  static void *write_block(uint8_t *page_offset, void *ptr,
                           uint32_t size_in_bytes) {
    std::memcpy(page_offset, ptr, size_in_bytes);
    return (uint8_t *)ptr + size_in_bytes;
  }

  void     write(uint32_t virt_addr, void *ptr, int64_t size_in_bytes);
  uint32_t ensure_page_exists(uint32_t addr);

  uint8_t read8(uint32_t off) const {
    return static_cast<const Derived *>(this)->read8(off);
  }

  uint16_t read16(uint32_t off) const {
    return static_cast<const Derived *>(this)->read16(off);
  }

  uint32_t read32(uint32_t off) const {
    return static_cast<const Derived *>(this)->read32(off);
  }

  uint64_t read64(uint32_t off) const {
    return static_cast<const Derived *>(this)->read64(off);
  }

  void write8(uint32_t off, uint8_t b) {
    static_cast<Derived *>(this)->write8(off, b);
  }

  void write16(uint32_t off, uint16_t h) {
    static_cast<Derived *>(this)->write16(off, h);
  }

  void write32(uint32_t off, uint32_t w) {
    static_cast<Derived *>(this)->write32(off, w);
  }

  void write64(uint32_t off, uint64_t dw) {
    static_cast<Derived *>(this)->write64(off, dw);
  }
};

template <typename Derived = void>
class sparse_memory_accessor : public sparse_memory_accessor_base<Derived> {
public:
  using sparse_memory_accessor_base<Derived>::sparse_memory_accessor_base;
  using sparse_memory_accessor_base<Derived>::ensure_page_exists;
  using sparse_memory_accessor_base<Derived>::write;
  using sparse_memory_accessor_base<Derived>::write_block;

  uint8_t  read8(uint32_t off) const;
  uint16_t read16(uint32_t off) const;
  uint32_t read32(uint32_t off) const;
  uint64_t read64(uint32_t off) const;

  void write8(uint32_t off, uint8_t b);
  void write16(uint32_t off, uint16_t h);
  void write32(uint32_t off, uint32_t w);
  void write64(uint32_t off, uint64_t dw);
};

/*
 * Acts as a crossbar switch CPU <=> Mem RW
 */
class address_router : public sparse_memory_accessor<address_router> {
  using sparse_memory_accessor<address_router>::sparse_memory_accessor;

private:
  uint32_t                            mtime_addr;
  uint32_t                            mtimecmp_addr;
  std::array<std::atomic<uint8_t>, 8> mtimecmp;

public:
  std::array<std::atomic<uint8_t>, 8> mtime;

  address_router(sparse_memory &mem, uint32_t mtime_addr,
                 uint32_t mtimecmp_addr);

  uint8_t read8(uint32_t off) const;
  void    write8(uint32_t off, uint8_t b);
};

template <typename Derived>
void sparse_memory_accessor_base<Derived>::write(uint32_t virt_addr, void *ptr,
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

template <typename Derived>
uint32_t
sparse_memory_accessor_base<Derived>::ensure_page_exists(uint32_t addr) {
  uint32_t key = addr & mask;
  if (!page.contains(key))
    page.emplace(key, std::make_unique<uint8_t[]>(page_size));
  return key;
}

template <typename Derived>
uint8_t sparse_memory_accessor<Derived>::read8(uint32_t off) const {
  {
    if (!this->page.contains(off & this->mask)) {
      return 0U;
    }
    return this->page.at(off & this->mask).get()[offset(off, 0U, 11U)];
  }
}

template <typename Derived>
uint16_t sparse_memory_accessor<Derived>::read16(uint32_t off) const {
  return read8(off) | (read8(off + 1) << 8);
}

template <typename Derived>
uint32_t sparse_memory_accessor<Derived>::read32(uint32_t off) const {
  return read16(off) | (read16(off + 2) << 16);
}

template <typename Derived>
uint64_t sparse_memory_accessor<Derived>::read64(uint32_t off) const {
  return (read32(off)) | (static_cast<uint64_t>(read32(off + 4)) << 32);
}

template <typename Derived>
void sparse_memory_accessor<Derived>::write8(uint32_t off, uint8_t b) {
  uint32_t key                                = this->ensure_page_exists(off);
  this->page[key].get()[offset(off, 0U, 11U)] = b;
}

template <typename Derived>
void sparse_memory_accessor<Derived>::write16(uint32_t off, uint16_t h) {
  write8(off, offset(h, 0U, 7U));
  write8(off + 1, offset(h, 8U, 15U));
}

template <typename Derived>
void sparse_memory_accessor<Derived>::write32(uint32_t off, uint32_t w) {
  write16(off, offset(w, 0U, 15U));
  write16(off + 2, offset(w, 16U, 31U));
}

template <typename Derived>
void sparse_memory_accessor<Derived>::write64(uint32_t off, uint64_t dw) {
  write32(off, (dw << 32) >> 32);
  write32(off + 4, dw >> 32);
}

} // namespace mem
#endif // MEMORY_SPARSE_MEMORY_HPP
